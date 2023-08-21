// Copyright (C) 2023 Cade Weinberg
//
// This file is part of Mint.
//
// Mint is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Mint is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Mint.  If not, see <http://www.gnu.org/licenses/>.
#include "adt/Environment.hpp"
#include "ast/definition/Definition.hpp"
#include "codegen/LLVMUtility.hpp"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"

namespace mint {
Environment::Environment(std::istream *in, std::ostream *out,
                         std::ostream *errout, std::ostream *log,
                         std::unique_ptr<llvm::LLVMContext> llvm_context,
                         std::unique_ptr<llvm::Module> llvm_module,
                         std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder,
                         llvm::TargetMachine *llvm_target_machine) noexcept
    : m_parser(this, in), m_input(in), m_output(out), m_error_output(errout),
      m_log_output(log), m_llvm_context(std::move(llvm_context)),
      m_llvm_module(std::move(llvm_module)),
      m_llvm_ir_builder(std::move(llvm_ir_builder)),
      m_llvm_target_machine(llvm_target_machine) {
  MINT_ASSERT(in != nullptr);
  MINT_ASSERT(out != nullptr);
  MINT_ASSERT(errout != nullptr);

  MINT_ASSERT(llvm_target_machine != nullptr);

  m_global_scope = Scope::createGlobalScope(getIdentifier(""));
  m_local_scope = m_global_scope;

  InitializeBuiltinBinops(this);
  InitializeBuiltinUnops(this);
}

//**** Environment Methods ****//
[[nodiscard]] auto Environment::nativeCPUFeatures() noexcept -> std::string {
  std::string features;
  llvm::StringMap<bool> map;

  if (llvm::sys::getHostCPUFeatures(map)) {
    auto cursor = map.begin();
    auto end = map.end();
    auto length = map.getNumItems();
    auto index = 0U;

    while (cursor != end) {
      if (cursor->getValue()) {
        features += "+";
      } else {
        features += "-";
      }

      features += cursor->getKeyData();

      if (index < (length - 1)) {
        features += ",";
      }

      ++cursor;
      ++index;
    }
  }

  return features;
}

[[nodiscard]] auto Environment::create(std::istream *in, std::ostream *out,
                                       std::ostream *errout,
                                       std::ostream *log) noexcept
    -> Environment {
  auto context = std::make_unique<llvm::LLVMContext>();
  auto target_triple = llvm::sys::getProcessTriple();

  std::string error_message;
  auto target =
      llvm::TargetRegistry::lookupTarget(target_triple, error_message);
  if (target == nullptr) {
    abort(error_message);
  }

  auto target_machine = target->createTargetMachine(
      target_triple, llvm::sys::getHostCPUName(), nativeCPUFeatures(),
      llvm::TargetOptions{}, llvm::Reloc::Model::PIC_,
      llvm::CodeModel::Model::Small);

  auto data_layout = target_machine->createDataLayout();
  auto ir_builder = std::make_unique<llvm::IRBuilder<>>(*context);
  auto llvm_module = std::make_unique<llvm::Module>("", *context);
  llvm_module->setDataLayout(data_layout);
  llvm_module->setTargetTriple(target_triple);

  return Environment{in,
                     out,
                     errout,
                     log,
                     std::move(context),
                     std::move(llvm_module),
                     std::move(ir_builder),
                     target_machine};
}

std::istream &Environment::getInputStream() noexcept { return *m_input; }

std::ostream &Environment::getOutputStream() noexcept { return *m_output; }

std::ostream &Environment::getErrorStream() noexcept { return *m_error_output; }

std::ostream &Environment::getLogStream() noexcept { return *m_log_output; }

fs::path &Environment::sourceFile() noexcept { return m_file; }

void Environment::sourceFile(fs::path const &file) noexcept { m_file = file; }

//**** Parser interface ****//
void Environment::setIStream(std::istream *in) noexcept {
  m_parser.setIstream(in);
}

auto Environment::extractSourceLine(Location const &location) const noexcept
    -> std::string_view {
  return m_parser.extractSourceLine(location);
}

void Environment::printErrorWithSource(Error const &error) const noexcept {
  m_parser.printErrorWithSource(*m_error_output, error);
}

auto Environment::endOfInput() const noexcept -> bool {
  return m_parser.endOfInput();
}
auto Environment::parse() -> Result<ast::Ptr> { return m_parser.parse(); }

//**** Scope Interface ****//
auto Environment::localScope() noexcept -> std::shared_ptr<Scope> {
  return m_local_scope;
}

auto Environment::nearestNamedScope() noexcept -> std::shared_ptr<Scope> {
  return m_local_scope->nearestNamedScope();
}

auto Environment::exchangeLocalScope(std::shared_ptr<Scope> scope) noexcept
    -> std::shared_ptr<Scope> {
  auto old_local = m_local_scope;
  m_local_scope = scope;
  return old_local;
}

void Environment::pushScope() noexcept {
  exchangeLocalScope(m_local_scope->pushScope());
}

void Environment::pushScope(Identifier name) noexcept {
  exchangeLocalScope(m_local_scope->pushScope(name));
}

void Environment::popScope() noexcept {
  exchangeLocalScope(m_local_scope->popScope());
}

//**** "module" interface ****/
void Environment::addAstToModule(ast::Ptr ast) noexcept {
  m_module.push_back(std::move(ast));
}

std::vector<ast::Ptr> &Environment::getModule() noexcept { return m_module; }

//**** DirectorySearcher interface ****/
void Environment::appendDirectory(fs::path file) noexcept {
  return m_directory_searcher.append(std::move(file));
}

auto Environment::fileExists(fs::path file) noexcept -> bool {
  return m_directory_searcher.exists(std::move(file));
}

auto Environment::fileSearch(fs::path file) noexcept
    -> std::optional<std::fstream> {
  return m_directory_searcher.search(std::move(file));
}

//**** Identifier Set Interface ****//
auto Environment::getIdentifier(std::string_view name) noexcept -> Identifier {
  return Identifier::create(&m_string_set, name);
}

auto Environment::getLambdaName() noexcept -> Identifier {
  static std::size_t count = 0U;
  std::string name{"l"};
  name += std::to_string(count++);
  return Identifier::create(&m_string_set, name);
}

//**** ImportSet interface ****/
auto Environment::alreadyImported(fs::path const &filename) noexcept -> bool {
  return m_import_set.contains(filename);
}

void Environment::addImport(fs::path const &filename) noexcept {
  m_import_set.insert(filename);
}

//**** Scope interface ****/

void Environment::unbindScope(Identifier name) noexcept {
  m_local_scope->unbindScope(name);
}

auto Environment::bindName(Identifier name, Attributes attributes,
                           type::Ptr type, ast::Ptr comptime_value,
                           llvm::Value *runtime_value) noexcept
    -> mint::Result<mint::Bindings::Binding> {
  return m_local_scope->bindName(name, attributes, type,
                                 std::move(comptime_value), runtime_value);
}

auto Environment::declareName(Identifier name, Attributes attributes,
                              type::Ptr type) noexcept
    -> mint::Result<mint::Bindings::Binding> {
  return m_local_scope->declareName(name, attributes, type);
}

auto Environment::lookupBinding(Identifier name) noexcept
    -> mint::Result<mint::Bindings::Binding> {
  return m_local_scope->lookupBinding(name);
}
auto Environment::lookupLocalBinding(Identifier name) noexcept
    -> mint::Result<mint::Bindings::Binding> {
  return m_local_scope->lookupLocalBinding(name);
}
auto Environment::qualifyName(Identifier name) noexcept -> Identifier {
  return m_local_scope->qualifyName(name);
}

//**** Use Before Def Interface ****/
std::optional<Error>
Environment::bindUseBeforeDef(Identifier undef, Identifier def,
                              std::shared_ptr<Scope> const &scope,
                              ir::Mir ir) noexcept {
  return m_use_before_def_map.bindUseBeforeDef(undef, def, scope,
                                               std::move(ir));
}

std::optional<Error> Environment::bindUseBeforeDef(Error const &error,
                                                   ast::Ptr ast) noexcept {
  return m_use_before_def_map.bindUseBeforeDef(error, std::move(ast));
}

std::optional<Error>
Environment::resolveTypeOfUseBeforeDef(Identifier def_name) noexcept {
  return m_use_before_def_map.resolveTypeOfUseBeforeDef(*this, def_name);
}

std::optional<Error>
Environment::resolveComptimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  return m_use_before_def_map.resolveComptimeValueOfUseBeforeDef(*this,
                                                                 def_name);
}

std::optional<Error>
Environment::resolveRuntimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  return m_use_before_def_map.resolveRuntimeValueOfUseBeforeDef(*this,
                                                                def_name);
}

//**** BinopTable Interface ****/
auto Environment::createBinop(Token op) noexcept -> BinopTable::Binop {
  return m_binop_table.emplace(op);
}
auto Environment::lookupBinop(Token op) noexcept
    -> std::optional<BinopTable::Binop> {
  return m_binop_table.lookup(op);
}

//**** UnopTable Interface ****/
auto Environment::createUnop(Token op) noexcept -> UnopTable::Unop {
  return m_unop_table.emplace(op);
}
auto Environment::lookupUnop(Token op) noexcept
    -> std::optional<UnopTable::Unop> {
  return m_unop_table.lookup(op);
}

//**** TypeInterner Interface ****/
auto Environment::getBooleanType() noexcept -> type::Ptr {
  return m_type_interner.getBooleanType();
}
auto Environment::getIntegerType() noexcept -> type::Ptr {
  return m_type_interner.getIntegerType();
}
auto Environment::getNilType() noexcept -> type::Ptr {
  return m_type_interner.getNilType();
}

auto Environment::getFunctionType(
    type::Ptr result_type, type::Function::Arguments argument_types) noexcept
    -> type::Ptr {
  return m_type_interner.getFunctionType(result_type,
                                         std::move(argument_types));
}

auto Environment::getLambdaType(type::Ptr function_type) noexcept -> type::Ptr {
  return m_type_interner.getLambdaType(function_type);
}

/**** LLVM interfaces ****/
/**** LLVM Helpers *****/
auto Environment::createBasicBlock(llvm::Twine const &name) noexcept
    -> llvm::BasicBlock * {
  return llvm::BasicBlock::Create(*m_llvm_context, name);
}

auto Environment::createBasicBlock(llvm::Function *function,
                                   llvm::Twine const &name) noexcept
    -> llvm::BasicBlock * {
  return llvm::BasicBlock::Create(*m_llvm_context, name, function);
}

auto Environment::hasInsertionPoint() const noexcept -> bool {
  return m_llvm_ir_builder->GetInsertBlock() != nullptr;
}

auto Environment::exchangeInsertionPoint(InsertionPoint point) noexcept
    -> InsertionPoint {
  InsertionPoint result{m_llvm_ir_builder->GetInsertBlock(),
                        m_llvm_ir_builder->GetInsertPoint()};
  if (!point.good())
    m_llvm_ir_builder->ClearInsertionPoint();
  else
    m_llvm_ir_builder->SetInsertPoint(point.basic_block, point.it);

  return result;
}

//**** String Set Interface ****//
auto Environment::internString(std::string_view string) noexcept
    -> std::string_view {
  return m_string_set.emplace(string);
}

//**** LLVM Module Interface ****//
auto Environment::getLLVMModule() noexcept -> llvm::Module & {
  return *m_llvm_module;
}

auto Environment::getOrInsertGlobal(std::string_view name,
                                    llvm::Type *type) noexcept
    -> llvm::GlobalVariable * {
  llvm::Constant *global =
      type->isFunctionTy()
          ? m_llvm_module->getOrInsertGlobal(name, getLLVMPointerType())
          : m_llvm_module->getOrInsertGlobal(name, type);

  return llvm::cast<llvm::GlobalVariable>(global);
}

// #TODO: add llvm attributes to the function
auto Environment::getOrInsertFunction(std::string_view name,
                                      llvm::FunctionType *type) noexcept
    -> llvm::FunctionCallee {
  return m_llvm_module->getOrInsertFunction(name, type);
}

//**** LLVM IRBuilder interface ****//
// types
auto Environment::getLLVMNilType() noexcept -> llvm::IntegerType * {
  return m_llvm_ir_builder->getInt1Ty();
}

auto Environment::getLLVMBooleanType() noexcept -> llvm::IntegerType * {
  return m_llvm_ir_builder->getInt1Ty();
}

auto Environment::getLLVMIntegerType() noexcept -> llvm::IntegerType * {
  return m_llvm_ir_builder->getInt32Ty();
}

auto Environment::getLLVMFunctionType(
    llvm::Type *result_type,
    llvm::ArrayRef<llvm::Type *> argument_types) noexcept
    -> llvm::FunctionType * {
  return llvm::FunctionType::get(result_type, argument_types,
                                 /* isVarArg = */ false);
}

auto Environment::getLLVMPointerType() noexcept -> llvm::PointerType * {
  return m_llvm_ir_builder->getPtrTy();
}

// values
auto Environment::getLLVMNil() noexcept -> llvm::ConstantInt * {
  return m_llvm_ir_builder->getInt1(false);
}

auto Environment::getLLVMBoolean(bool value) noexcept -> llvm::ConstantInt * {
  return m_llvm_ir_builder->getInt1(value);
}

auto Environment::getLLVMInteger(int value) noexcept -> llvm::ConstantInt * {
  return m_llvm_ir_builder->getInt32(value);
}

// instructions
llvm::Value *Environment::createLLVMNeg(llvm::Value *right,
                                        llvm::Twine const &name,
                                        bool no_unsigned_wrap,
                                        bool no_signed_wrap) noexcept {
  return m_llvm_ir_builder->CreateNeg(right, name, no_unsigned_wrap,
                                      no_signed_wrap);
}

llvm::Value *Environment::createLLVMNot(llvm::Value *right,
                                        llvm::Twine const &name) noexcept {
  return m_llvm_ir_builder->CreateNot(right, name);
}

/* https://llvm.org/docs/LangRef.html#add-instruction */
llvm::Value *Environment::createLLVMAdd(llvm::Value *left, llvm::Value *right,
                                        llvm::Twine const &name,
                                        bool no_unsigned_wrap,
                                        bool no_signed_wrap) noexcept {
  return m_llvm_ir_builder->CreateAdd(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
}

/* https://llvm.org/docs/LangRef.html#sub-instruction */
llvm::Value *Environment::createLLVMSub(llvm::Value *left, llvm::Value *right,
                                        llvm::Twine const &name,
                                        bool no_unsigned_wrap,
                                        bool no_signed_wrap) noexcept {
  return m_llvm_ir_builder->CreateSub(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
}

/* https://llvm.org/docs/LangRef.html#mul-instruction */
llvm::Value *Environment::createLLVMMul(llvm::Value *left, llvm::Value *right,
                                        llvm::Twine const &name,
                                        bool no_unsigned_wrap,
                                        bool no_signed_wrap) noexcept {
  return m_llvm_ir_builder->CreateMul(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
}

/* https://llvm.org/docs/LangRef.html#sdiv-instruction */
llvm::Value *Environment::createLLVMSDiv(llvm::Value *left, llvm::Value *right,
                                         llvm::Twine const &name,
                                         bool is_exact) noexcept {
  return m_llvm_ir_builder->CreateSDiv(left, right, name, is_exact);
}

/* https://llvm.org/docs/LangRef.html#srem-instruction */
llvm::Value *Environment::createLLVMSRem(llvm::Value *left, llvm::Value *right,
                                         llvm::Twine const &name) noexcept {
  return m_llvm_ir_builder->CreateSRem(left, right, name);
}

/* https://llvm.org/docs/LangRef.html#icmp-instruction */
auto Environment::createLLVMICmpEQ(llvm::Value *left, llvm::Value *right,
                                   const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpEQ(left, right, name);
}

auto Environment::createLLVMICmpNE(llvm::Value *left, llvm::Value *right,
                                   const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpNE(left, right, name);
}

auto Environment::createLLVMICmpSGT(llvm::Value *left, llvm::Value *right,
                                    const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpSGT(left, right, name);
}

auto Environment::createLLVMICmpSGE(llvm::Value *left, llvm::Value *right,
                                    const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpSGE(left, right, name);
}

auto Environment::createLLVMICmpSLT(llvm::Value *left, llvm::Value *right,
                                    const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpSLT(left, right, name);
}

auto Environment::createLLVMICmpSLE(llvm::Value *left, llvm::Value *right,
                                    const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateICmpSLE(left, right, name);
}

auto Environment::createLLVMAnd(llvm::Value *left, llvm::Value *right,
                                const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateAnd(left, right, name);
}

auto Environment::createLLVMOr(llvm::Value *left, llvm::Value *right,
                               const llvm::Twine &name) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateOr(left, right, name);
}

// memory accessors
auto Environment::createLLVMLoad(llvm::Type *type, llvm::Value *source) noexcept
    -> llvm::Value * {
  return m_llvm_ir_builder->CreateLoad(type, source);
}

// function instructions
auto Environment::createLLVMCall(llvm::Function *callee,
                                 llvm::ArrayRef<llvm::Value *> arguments,
                                 llvm::Twine const &name,
                                 llvm::MDNode *fp_math_tag) noexcept
    -> llvm::CallInst * {
  return m_llvm_ir_builder->CreateCall(callee, arguments, name, fp_math_tag);
}

auto Environment::createLLVMCall(llvm::FunctionType *type, llvm::Value *value,
                                 llvm::ArrayRef<llvm::Value *> arguments,
                                 llvm::Twine const &name,
                                 llvm::MDNode *fp_math_tag) noexcept
    -> llvm::CallInst * {
  return m_llvm_ir_builder->CreateCall(type, value, arguments, name,
                                       fp_math_tag);
}

auto Environment::createLLVMReturn(llvm::Value *value) noexcept
    -> llvm::ReturnInst * {
  if (value == nullptr)
    return m_llvm_ir_builder->CreateRetVoid();
  else
    return m_llvm_ir_builder->CreateRet(value);
}

} // namespace mint
