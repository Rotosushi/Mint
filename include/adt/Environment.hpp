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
#pragma once
#include <iostream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"

#include "adt/AstAllocator.hpp"
#include "adt/BinopTable.hpp"
#include "adt/DirectorySearch.hpp"
#include "adt/Identifier.hpp"
#include "adt/ImportSet.hpp"
#include "adt/Scope.hpp"
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"
#include "adt/UseBeforeDefMap.hpp"
#include "scan/Parser.hpp"

namespace mint {
class Environment {
  DirectorySearcher directory_searcher;
  ImportSet import_set;
  IdentifierSet identifier_set;
  TypeInterner type_interner;
  AstAllocator ast_allocator;
  BinopTable binop_table;
  UnopTable unop_table;
  UseBeforeDefMap use_before_def_map;
  // #NOTE: since we only have std::weak_ptrs
  // back up the tree of scopes, we must hold
  // a shared_ptr to the top of the tree.
  // so when we traverse to a new scope,
  // all of the scopes stay alive.
  std::shared_ptr<Scope> global_scope;
  std::shared_ptr<Scope> local_scope;
  Parser parser;
  std::vector<ast::Ptr> m_module;

  std::istream *in;
  std::ostream *out;
  std::ostream *errout;

  std::unique_ptr<llvm::LLVMContext> llvm_context;
  std::unique_ptr<llvm::Module> llvm_module;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder;
  llvm::TargetMachine *llvm_target_machine;
  // llvm::Function *current_llvm_function;

  Environment(std::istream *in, std::ostream *out, std::ostream *errout,
              std::unique_ptr<llvm::LLVMContext> llvm_context,
              std::unique_ptr<llvm::Module> llvm_module,
              std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder,
              llvm::TargetMachine *llvm_target_machine) noexcept
      : identifier_set(), binop_table(), unop_table(), use_before_def_map(),
        global_scope(Scope::createGlobalScope()), local_scope(global_scope),
        parser(this, in), in(in), out(out), errout(errout),
        llvm_context(std::move(llvm_context)),
        llvm_module(std::move(llvm_module)),
        llvm_ir_builder(std::move(llvm_ir_builder)),
        llvm_target_machine(llvm_target_machine)
  // ,current_llvm_function(nullptr)
  {
    MINT_ASSERT(in != nullptr);
    MINT_ASSERT(out != nullptr);
    MINT_ASSERT(errout != nullptr);

    MINT_ASSERT(llvm_target_machine != nullptr);

    InitializeBuiltinBinops(this);
    InitializeBuiltinUnops(this);
  }

public:
  [[nodiscard]] static auto nativeCPUFeatures() noexcept -> std::string;

  [[nodiscard]] static auto create(std::istream *in = &std::cin,
                                   std::ostream *out = &std::cout,
                                   std::ostream *errout = &std::cerr) noexcept
      -> Environment;

  // #TODO: reorganize environment methods more clearly into
  // groups. according to which member they access.

  auto repl() noexcept -> int;

  auto compile(std::filesystem::path file) noexcept -> int;

  void printErrorWithSource(Error const &error) const noexcept {
    if (error.isDefault()) {
      auto &data = error.getDefault();
      auto bad_source = parser.extractSourceLine(data.location);
      error.print(*errout, bad_source);
    } else {
      error.print(*errout);
    }
  }

  void printErrorWithSource(Error const &error,
                            Parser const &parser) const noexcept {
    if (error.isDefault()) {
      auto &data = error.getDefault();
      auto bad_source = parser.extractSourceLine(data.location);
      error.print(*errout, bad_source);
    } else {
      error.print(*errout);
    }
  }

  void addAstToModule(ast::Ptr ast) noexcept { m_module.push_back(ast); }

  /*
    #NOTE:
    string is some "\"...\""
    return "..."
  */
  auto getText(std::string_view string) noexcept -> std::string_view {
    auto cursor = std::next(string.begin());
    auto end = std::prev(string.end());

    // #TODO: replace escape sequences with character literals?

    return {cursor, static_cast<std::size_t>(std::distance(cursor, end))};
  }

  void appendDirectory(fs::path file) noexcept {
    return directory_searcher.append(std::move(file));
  }

  auto fileExists(fs::path file) noexcept {
    return directory_searcher.exists(std::move(file));
  }

  auto fileSearch(fs::path file) noexcept {
    return directory_searcher.search(std::move(file));
  }

  auto getIdentifier(std::string_view name) noexcept {
    return identifier_set.emplace(name);
  }

  auto alreadyImported(fs::path const &filename) noexcept -> bool {
    return import_set.contains(filename);
  }

  void addImport(fs::path const &filename) noexcept {
    import_set.insert(filename);
  }

  auto emitLLVMIR(fs::path filename) noexcept -> int {
    auto llvm_ir_filename = filename.replace_extension("ll");
    std::error_code error;
    llvm::raw_fd_ostream outfile{llvm_ir_filename.c_str(), error};
    if (error) {
      *errout << "Couldn't open file: " << llvm_ir_filename << " -- " << error
              << "\n";
      return EXIT_FAILURE;
    }

    llvm_module->print(outfile, /* AssemblyAnnotationWriter = */ nullptr);
    return EXIT_SUCCESS;
  }

  auto localScope() noexcept -> std::shared_ptr<Scope> { return local_scope; }

  /*
    intended to be called when processing an Ast::Function,
    such that lookup and binding occurs within the local
    scope of the function.
    note the lack of a scope name, and thus no ability to
    bind the scope to the current scope. thus anonymous scopes
    are only alive as long as they are the local_scope.

    this behavior is intended for functions, whose names
    only need to be alive for the time functions are being
    evaluated. but not for modules, as a modules names
    must be alive for the lifetime of a given program.
  */
  void pushScope() noexcept {
    local_scope = Scope::createScope({}, local_scope);
  }

  /*
    called when processing an Ast::Module,
    such that lookup and binding occurs within the
    local scope of the module.
  */
  void pushScope(Identifier name) noexcept {
    auto found = local_scope->lookupScope(name);
    if (found) {
      local_scope = found.value().ptr();
      return;
    }

    auto new_scope = local_scope->bindScope(name);
    local_scope = new_scope.ptr();
  }
  // called when we fail to create a module, so we
  // don't partially define a module within the current
  // namespace.
  void unbindScope(Identifier name) noexcept { local_scope->unbindScope(name); }

  // traverse up the scope tree one level.
  void popScope() noexcept {
    if (local_scope->isGlobal()) {
      return; // cannot traverse past global scope
    }

    local_scope = local_scope->getPrevScope();
  }

  /*
    #NOTE: called when we just encountered a term that could not
    be type'd because it used a name before that name was defined.
  */
  std::optional<Error> bindUseBeforeDef(const Error &error,
                                        const ast::Ptr &ast) noexcept;

  /*
    #NOTE: called when we successfully typecheck a new definition.
    creates partial bindings for any definitions that are in the
    use-before-def-map relying on the given definition.
  */
  std::optional<Error> resolveTypeOfUseBeforeDef(Identifier def) noexcept;

  /*
  #NOTE: called when we successfully evaluate a new definition.
  creates full bindings for any definition that is in the
  use-before-def map relying on the given definition.
*/
  std::optional<Error>
  resolveComptimeValueOfUseBeforeDef(Identifier def) noexcept;

  std::optional<Error>
  resolveRuntimeValueOfUseBeforeDef(Identifier def) noexcept;

  /*
    #NOTE: undef is the name which caused the use-before-def error.
      definition is the name of the definition which failed to typecheck.
      that is, undef is the name which needs to be defined for the
      definition to be able to typecheck. (or at least, make it past this
      single use-before-def type error.)
  */
  void bindUseBeforeDef(Identifier undef, Identifier definition, ast::Ptr ast,
                        std::shared_ptr<Scope> scope) noexcept {
    use_before_def_map.insert(undef, definition, std::move(ast), scope);
  }
  [[nodiscard]] auto lookupUseBeforeDef(Identifier undef) noexcept
      -> UseBeforeDefMap::Range {
    return use_before_def_map.lookup(undef);
  }

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept {
    return local_scope->bindName(name, attributes, type, comptime_value,
                                 runtime_value);
  }

  auto partialBindName(Identifier name, Attributes attributes,
                       type::Ptr type) noexcept {
    return local_scope->partialBindName(name, attributes, type);
  }

  auto lookupBinding(Identifier name) noexcept {
    return local_scope->lookup(name);
  }
  auto getQualifiedName(Identifier name) noexcept {
    return local_scope->getQualifiedName(name);
  }
  /* https://llvm.org/docs/LangRef.html#identifiers */
  auto getQualifiedNameForLLVM(Identifier name) noexcept -> Identifier;

  auto createBinop(Token op) noexcept { return binop_table.emplace(op); }
  auto lookupBinop(Token op) noexcept { return binop_table.lookup(op); }

  auto createUnop(Token op) noexcept { return unop_table.emplace(op); }
  auto lookupUnop(Token op) noexcept { return unop_table.lookup(op); }

  auto getBooleanType() noexcept { return type_interner.getBooleanType(); }
  auto getIntegerType() noexcept { return type_interner.getIntegerType(); }
  auto getNilType() noexcept { return type_interner.getNilType(); }

  auto getLetAst(Attributes attributes, Location location,
                 std::optional<type::Ptr> annotation, Identifier name,
                 ast::Ptr ast) noexcept -> ast::Ptr {
    return ast_allocator.createLet(attributes, location, annotation, name,
                                   std::move(ast));
  }

  auto getNilAst(Attributes attributes, Location location) noexcept
      -> ast::Ptr {
    return ast_allocator.createNil(attributes, location);
  }

  auto getBooleanAst(Attributes attributes, Location location,
                     bool value) noexcept -> ast::Ptr {
    return ast_allocator.createBoolean(attributes, location, value);
  }

  auto getIntegerAst(Attributes attributes, Location location,
                     int value) noexcept -> ast::Ptr {
    return ast_allocator.createInteger(attributes, location, value);
  }

  auto getParensAst(Attributes attributes, Location location,
                    ast::Ptr ast) noexcept -> ast::Ptr {
    return ast_allocator.createParens(attributes, location, std::move(ast));
  }

  auto getAffixAst(Attributes attributes, Location location,
                   ast::Ptr ast) noexcept -> ast::Ptr {
    return ast_allocator.createAffix(attributes, location, std::move(ast));
  }

  auto getBinopAst(Attributes attributes, Location location, Token op,
                   ast::Ptr left, ast::Ptr right) noexcept -> ast::Ptr {
    return ast_allocator.createBinop(attributes, location, op, std::move(left),
                                     std::move(right));
  }

  auto getImportAst(Attributes attributes, Location location,
                    std::string filename) noexcept -> ast::Ptr {
    return ast_allocator.createImport(attributes, location,
                                      std::move(filename));
  }

  auto getModuleAst(Attributes attributes, Location location, Identifier name,
                    ast::Module::Expressions expressions) noexcept -> ast::Ptr {
    return ast_allocator.createModule(attributes, location, name,
                                      std::move(expressions));
  }

  auto getUnopAst(Attributes attributes, Location location, Token op,
                  ast::Ptr right) noexcept -> ast::Ptr {
    return ast_allocator.createUnop(attributes, location, op, std::move(right));
  }

  auto getVariableAst(Attributes attributes, Location location,
                      Identifier name) noexcept -> ast::Ptr {
    return ast_allocator.createVariable(attributes, location, name);
  }

  /* LLVM interface */
  // types
  auto getLLVMNilType() noexcept -> llvm::IntegerType * {
    return llvm_ir_builder->getInt1Ty();
  }

  auto getLLVMBooleanType() noexcept -> llvm::IntegerType * {
    return llvm_ir_builder->getInt1Ty();
  }

  auto getLLVMIntegerType() noexcept -> llvm::IntegerType * {
    return llvm_ir_builder->getInt32Ty();
  }

  // values
  auto getLLVMNil() noexcept -> llvm::ConstantInt * {
    return llvm_ir_builder->getInt1(false);
  }

  auto getLLVMBoolean(bool value) noexcept -> llvm::ConstantInt * {
    return llvm_ir_builder->getInt1(value);
  }

  auto getLLVMInteger(int value) noexcept -> llvm::ConstantInt * {
    return llvm_ir_builder->getInt32(value);
  }

  // instructions
  llvm::Value *createLLVMNeg(llvm::Value *right, llvm::Twine const &name = "",
                             bool no_unsigned_wrap = false,
                             bool no_signed_wrap = false) noexcept {
    return llvm_ir_builder->CreateNeg(right, name, no_unsigned_wrap,
                                      no_signed_wrap);
  }

  llvm::Value *createLLVMNot(llvm::Value *right, llvm::Twine const &name = "") {
    return llvm_ir_builder->CreateNot(right, name);
  }

  /* https://llvm.org/docs/LangRef.html#add-instruction */
  llvm::Value *createLLVMAdd(llvm::Value *left, llvm::Value *right,
                             llvm::Twine const &name = "",
                             bool no_unsigned_wrap = false,
                             bool no_signed_wrap = false) noexcept {
    return llvm_ir_builder->CreateAdd(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
  }

  /* https://llvm.org/docs/LangRef.html#sub-instruction */
  llvm::Value *createLLVMSub(llvm::Value *left, llvm::Value *right,
                             llvm::Twine const &name = "",
                             bool no_unsigned_wrap = false,
                             bool no_signed_wrap = false) noexcept {
    return llvm_ir_builder->CreateSub(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
  }

  /* https://llvm.org/docs/LangRef.html#mul-instruction */
  llvm::Value *createLLVMMul(llvm::Value *left, llvm::Value *right,
                             llvm::Twine const &name = "",
                             bool no_unsigned_wrap = false,
                             bool no_signed_wrap = false) noexcept {
    return llvm_ir_builder->CreateMul(left, right, name, no_unsigned_wrap,
                                      no_signed_wrap);
  }

  /* https://llvm.org/docs/LangRef.html#sdiv-instruction */
  llvm::Value *createLLVMSDiv(llvm::Value *left, llvm::Value *right,
                              llvm::Twine const &name = "",
                              bool is_exact = false) noexcept {
    return llvm_ir_builder->CreateSDiv(left, right, name, is_exact);
  }

  /* https://llvm.org/docs/LangRef.html#srem-instruction */
  llvm::Value *createLLVMSRem(llvm::Value *left, llvm::Value *right,
                              llvm::Twine const &name = "") noexcept {
    return llvm_ir_builder->CreateSRem(left, right, name);
  }

  /* https://llvm.org/docs/LangRef.html#icmp-instruction */
  auto createLLVMICmpEQ(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpEQ(left, right, name);
  }

  auto createLLVMICmpNE(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpNE(left, right, name);
  }

  auto createLLVMICmpSGT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpSGT(left, right, name);
  }

  auto createLLVMICmpSGE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpSGE(left, right, name);
  }

  auto createLLVMICmpSLT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpSLT(left, right, name);
  }

  auto createLLVMICmpSLE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateICmpSLE(left, right, name);
  }

  auto createLLVMAnd(llvm::Value *left, llvm::Value *right,
                     const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateAnd(left, right, name);
  }

  auto createLLVMOr(llvm::Value *left, llvm::Value *right,
                    const llvm::Twine &name = "") noexcept {
    return llvm_ir_builder->CreateOr(left, right, name);
  }

  /* allocations */
  auto createLLVMGlobalVariable(std::string_view name, llvm::Type *type,
                                llvm::Constant *init = nullptr) noexcept
      -> llvm::GlobalVariable *;

  /* composite 'instructions' */
  auto createLLVMLoad(llvm::Type *type, llvm::Value *source) -> llvm::Value * {
    // #NOTE: we can only load single value types
    // all types currently available in the language
    // are single value. so we need not worry.
    return llvm_ir_builder->CreateLoad(type, source);
  }
};
} // namespace mint
