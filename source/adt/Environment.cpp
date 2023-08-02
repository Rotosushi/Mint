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
#include "utility/LLVMUtility.hpp"

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

      if (auto failed = bindUseBeforeDef(error, std::move(ast))) {
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

    addAstToModule(std::move(ast));
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
//  #TODO: I think we can handle multiple source files by "import"ing
//  each subsequent file into the environment created by the first
//  file given. then generating the code from there. this might
//  convert straight to a multithreaded approach, where a thread is
//  launched per input file.
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

  //   codegen each term within the module,
  //   this populates the llvm_module with
  //   the llvm equivalent of all terms.
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

void Environment::pushScope() noexcept {
  m_local_scope = Scope::createScope({}, m_local_scope);
}

void Environment::pushScope(Identifier name) noexcept {
  auto found = m_local_scope->lookupScope(name);
  if (found) {
    m_local_scope = found.value().ptr();
    return;
  }

  auto new_scope = m_local_scope->bindScope(name);
  m_local_scope = new_scope.ptr();
}

void Environment::popScope() noexcept {
  if (m_local_scope->isGlobal()) {
    return; // cannot traverse past global scope
  }

  m_local_scope = m_local_scope->getPrevScope();
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
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.undef;
  auto ubd_def_name = names.def;
  auto scope_name = scope->qualifiedName();

  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  m_use_before_def_map.insert(ubd_name, ubd_def_name, scope_name,
                              std::move(ast), scope);
  return std::nullopt;
}

std::optional<Error>
Environment::bindUseBeforeDef(UseBeforeDefMap::Elements &elements,
                              Error const &error, ast::Ptr ast) noexcept {
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.undef;
  auto ubd_def_name = names.def;
  auto scope_name = scope->qualifiedName();

  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  elements.emplace_back(ubd_name, ubd_def_name, scope_name, std::move(ast),
                        scope);
  return std::nullopt;
}

std::optional<Error>
Environment::resolveTypeOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    // #NOTE: we enter the local scope of the ubd definition
    // so we know that when we construct it's binding we
    // construct it in the correct scope.
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    // #NOTE: since we are resolving the ubd here, we can clear the error
    ubd_def_ast->clearUseBeforeDef();

    auto result = ubd_def_ast->typecheck(*this);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        it.being_resolved(false);
        exchangeLocalScope(old_local_scope);
        return error;
      }

      // handle another use before def error.
      bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
      old_entries.append(it);
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    m_use_before_def_map.erase(old_entries);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error>
Environment::resolveComptimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->evaluate(*this);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        it.being_resolved(false);
        exchangeLocalScope(old_local_scope);
        return error;
      }

      // handle another use before def error.
      bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
      old_entries.append(it);
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    m_use_before_def_map.erase(old_entries);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error>
Environment::resolveRuntimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->codegen(*this);
    if (!result) {
      return result.error();
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  // #NOTE: codegen is the last step when processing an ast.
  // so we can safely remove these entries from the ubd map
  m_use_before_def_map.erase(range);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
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

/**** LLVM IRBuilder interface ****/
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

/**** composite llvm IR 'instructions' ****/
// allocations
auto Environment::createLLVMGlobalVariable(std::string_view name,
                                           llvm::Type *type,
                                           llvm::Constant *init) noexcept
    -> llvm::GlobalVariable * {
  auto variable = llvm::cast<llvm::GlobalVariable>(
      m_llvm_module->getOrInsertGlobal(name, type));

  if (init != nullptr)
    variable->setInitializer(init);

  return variable;
}

// loads/stores
auto Environment::createLLVMLoad(llvm::Type *type, llvm::Value *source)
    -> llvm::Value * {
  // #NOTE: we can only load single value types.
  // however all types currently available in
  // the language are single value.
  return m_llvm_ir_builder->CreateLoad(type, source);
}

} // namespace mint
