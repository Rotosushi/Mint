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
    : m_global_scope(Scope::createGlobalScope(m_identifier_set.empty_id())),
      m_local_scope(m_global_scope), m_parser(this, in), m_input(in),
      m_output(out), m_error_output(errout), m_log_output(log),
      m_llvm_context(std::move(llvm_context)),
      m_llvm_module(std::move(llvm_module)),
      m_llvm_ir_builder(std::move(llvm_ir_builder)),
      m_llvm_target_machine(llvm_target_machine) {
  MINT_ASSERT(in != nullptr);
  MINT_ASSERT(out != nullptr);
  MINT_ASSERT(errout != nullptr);

  MINT_ASSERT(llvm_target_machine != nullptr);

  InitializeBuiltinBinops(this);
  InitializeBuiltinUnops(this);
}

/**** Environment Methods ****/

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

auto Environment::getTextFromTextLiteral(std::string_view string) noexcept
    -> std::string_view {
  auto cursor = std::next(string.begin());
  auto end = std::prev(string.end());

  // #TODO: replace escape sequences with character literals here?

  return {cursor, static_cast<std::size_t>(std::distance(cursor, end))};
}

auto Environment::repl(bool do_print) noexcept -> int {
  while (true) {
    if (do_print)
      *m_output << "# ";

    auto parse_result = m_parser.parse();
    if (!parse_result) {
      auto &error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;

      printErrorWithSource(error);
      continue;
    }
    auto &ast = parse_result.value();

    auto typecheck_result = ast->typecheck(*this);
    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (!error.isUseBeforeDef()) {
        printErrorWithSource(error);
        continue;
      }

      if (auto failed = bindUseBeforeDef(error, ast)) {
        printErrorWithSource(failed.value());
      }
      continue;
    }
    auto &type = typecheck_result.value();

    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      printErrorWithSource(evaluate_result.error());
      continue;
    }
    auto &value = evaluate_result.value();

    if (do_print)
      *m_output << ast << " : " << type << " => " << value << "\n";

    addAstToModule(ast);
  }

  return EXIT_SUCCESS;
}

//  Parse, Typecheck, and Evaluate each ast within the source file
//  then Codegen all of the terms collected and emit all of that
//  as LLVM IR.
//
//  #TODO: emit the terms into a LLVM bitcode, or object file.
//
//  #TODO: add a link step to produce a library, or executable
//
//  #TODO: I think we can handle multiple source files by "repl"ing
//  each subsequent file into the environment created by the first
//  file given. then generating the code after all files are processed.
//  this might convert to a multithreaded approach, where a thread is
//  launched per input file. but we would need some way of
//  A) bringing all of the results into a single output file and
//  B) something else I am sure I haven't thought of.
auto Environment::compile(fs::path filename) noexcept -> int {
  auto found = fileSearch(filename);
  if (!found) {
    Error e{Error::Kind::FileNotFound, Location{}, filename.c_str()};
    e.print(*m_error_output);
    return EXIT_FAILURE;
  }
  auto &file = found.value();
  m_parser.setIstream(&file);

  repl(/* do_print = */ false);

  for (auto &ast : m_module) {
    auto codegen_result = ast->codegen(*this);
    if (!codegen_result) {
      printErrorWithSource(codegen_result.error());
      return EXIT_FAILURE;
    }
  }

  emitLLVMIR(*m_llvm_module, filename, *m_error_output);
  return EXIT_SUCCESS;
}

void Environment::printErrorWithSource(Error const &error) const noexcept {
  if (error.isDefault()) {
    auto &data = error.getDefault();
    auto bad_source = m_parser.extractSourceLine(data.location);
    error.print(*m_error_output, bad_source);
  } else {
    error.print(*m_error_output);
  }
}

void Environment::printErrorWithSource(Error const &error,
                                       Parser const &parser) const noexcept {
  if (error.isDefault()) {
    auto &data = error.getDefault();
    auto bad_source = parser.extractSourceLine(data.location);
    error.print(*m_error_output, bad_source);
  } else {
    error.print(*m_error_output);
  }
}

auto Environment::localScope() noexcept -> std::shared_ptr<Scope> {
  return m_local_scope;
}

auto Environment::exchangeLocalScope(std::shared_ptr<Scope> scope) noexcept
    -> std::shared_ptr<Scope> {
  auto old_local = m_local_scope;
  m_local_scope = scope;
  return old_local;
}

shared_ptr<Scope> Environment::pushScope() noexcept {
  return exchangeLocalScope(Scope::createScope({}, m_local_scope.get()));
}

shared_ptr<Scope> Environment::pushScope(Identifier name) noexcept {
  auto found = m_local_scope->lookupScope(name);
  if (found) { 
    return found.value().ptr();
  }

  auto new_scope = m_local_scope->bindScope(name);
  m_local_scope = new_scope.ptr();
}

shared_ptr<Scope> Environment::popScope() noexcept {
  if (m_local_scope->isGlobal()) {
    return; // cannot traverse past global scope
  }

  // #TODO: somehow m_local_scope->m_prev_scope 
  // points to a corrupted previous scope.
  // not nullptr, but a random address.
  m_local_scope = m_local_scope->prevScope();
}

//**** "module" interface ****/
void Environment::addAstToModule(ast::Ptr ast) noexcept {
  m_module.push_back(std::move(ast));
}

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

auto Environment::getIdentifier(std::string_view name) noexcept -> Identifier {
  return m_identifier_set.emplace(name);
}

//**** String Set Interface ****//
auto Environment::internString(std::string_view string) noexcept
    -> std::string_view {
  return m_string_set.emplace(string);
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

auto Environment::partialBindName(Identifier name, Attributes attributes,
                                  type::Ptr type) noexcept
    -> mint::Result<mint::Bindings::Binding> {
  return m_local_scope->partialBindName(name, attributes, type);
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
auto Environment::getBooleanType() noexcept -> type::Boolean const * {
  return m_type_interner.getBooleanType();
}
auto Environment::getIntegerType() noexcept -> type::Integer const * {
  return m_type_interner.getIntegerType();
}
auto Environment::getNilType() noexcept -> type::Nil const * {
  return m_type_interner.getNilType();
}

auto Environment::getLambdaType(type::Ptr result_type,
                                type::Lambda::Arguments argument_types) noexcept
    -> type::Lambda const * {
  return m_type_interner.getLambdaType(result_type, std::move(argument_types));
}

/**** LLVM interface ****/
/**** LLVM Helpers *****/
/* https://llvm.org/docs/LangRef.html#identifiers */
auto Environment::createQualifiedNameForLLVM(Identifier name) noexcept
    -> Identifier {
  auto qualified = qualifyName(name);
  auto view = qualified.view();
  std::string llvm_name;

  // the llvm_name is the same as the given name,
  // where "::" is replaced with "."
  auto cursor = view.begin();
  auto end = view.end();
  while (cursor != end) {
    auto c = *cursor;
    if (c == ':') {
      llvm_name += '.';
      ++cursor; // eat "::"
      ++cursor;
    } else {
      llvm_name += c;
      ++cursor; // eat the char
    }
  }

  return m_identifier_set.emplace(std::move(llvm_name));
}

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
  return m_llvm_ir_builder->GetInsertBlock() == nullptr;
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

//**** LLVM Module Interface ****//
auto Environment::getOrInsertGlobal(std::string_view name,
                                    llvm::Type *type) noexcept
    -> llvm::GlobalVariable * {
  return llvm::cast<llvm::GlobalVariable>(
      m_llvm_module->getOrInsertGlobal(name, type));
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

auto Environment::createLLVMReturn(llvm::Value *value) noexcept
    -> llvm::ReturnInst * {
  if (value == nullptr)
    return m_llvm_ir_builder->CreateRetVoid();
  else
    return m_llvm_ir_builder->CreateRet(value);
}

} // namespace mint
