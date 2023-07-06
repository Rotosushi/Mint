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

#include "adt/BinopTable.hpp"
#include "adt/DirectorySearch.hpp"
#include "adt/Identifier.hpp"
#include "adt/Scope.hpp"
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"
#include "adt/UseBeforeDefMap.hpp"
#include "ast/All.hpp"
#include "scan/Parser.hpp"

namespace mint {
class Environment {
  DirectorySearcher directory_searcher;
  IdentifierSet identifier_set;
  TypeInterner type_interner;
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

  std::istream *in;
  std::ostream *out;
  std::ostream *errout;

  std::unique_ptr<llvm::LLVMContext> llvm_context;
  std::unique_ptr<llvm::Module> llvm_module;
  std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder;
  llvm::TargetMachine *llvm_target_machine;
  llvm::Function *current_llvm_function;

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
        llvm_target_machine(llvm_target_machine),
        current_llvm_function(nullptr) {
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

  auto repl() noexcept -> int;

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

  /*
    string is some "\"...\""
    return "..."
  */
  auto getText(std::string_view string) noexcept -> std::string_view {
    auto cursor = std::next(string.begin());
    auto end = std::prev(string.end());

    // #TODO: when do we replace escape sequences with character literals?

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
  std::optional<Error> handleUseBeforeDef(const Error &error,
                                          ast::Ptr &ast) noexcept;

  /*
    #NOTE: called when we just defined a new name. Resolves UseBeforeDef
    bindings that failed to type due to the given definition not being
    present.
    this assumes that the "Identifier def" was just fully defined.
  */
  std::optional<Error> resolveUseBeforeDef(Identifier def) noexcept;

  /*
    #NOTE: undef is the name which caused the use-before-def error.
      definition is the name of the definition which failed to typecheck.
      that is, undef is the name which needs to be defined for the
      definition to be able to typecheck. (or at least, make it past this
      single use-before-def type error.)
  */
  void bindUseBeforeDef(Identifier undef, Identifier definition, ast::Ptr ast) {
    use_before_def_map.insert(undef, definition, std::move(ast));
  }
  [[nodiscard]] auto lookupUseBeforeDef(Identifier undef)
      -> UseBeforeDefMap::Range {
    return use_before_def_map.lookup(undef);
  }

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr value) noexcept {
    return local_scope->bindName(name, attributes, type, value);
  }

  auto lookup(Identifier name) { return local_scope->lookup(name); }
  auto getQualifiedName(Identifier name) {
    return local_scope->getQualifiedName(name);
  }

  auto createBinop(Token op) { return binop_table.emplace(op); }
  auto lookupBinop(Token op) { return binop_table.lookup(op); }

  auto createUnop(Token op) { return unop_table.emplace(op); }
  auto lookupUnop(Token op) { return unop_table.lookup(op); }

  auto getBooleanType() noexcept { return type_interner.getBooleanType(); }
  auto getIntegerType() noexcept { return type_interner.getIntegerType(); }
  auto getNilType() noexcept { return type_interner.getNilType(); }

  auto getLetAst(Attributes attributes, Location location,
                 std::optional<type::Ptr> annotation, Identifier name,
                 ast::Ptr ast) noexcept -> ast::Ptr {
    return ast::Let::create(attributes, location, annotation, name,
                            std::move(ast));
  }

  auto getNilAst(Attributes attributes, Location location) noexcept
      -> ast::Ptr {
    return ast::Nil::create(attributes, location);
  }

  auto getBooleanAst(Attributes attributes, Location location,
                     bool value) noexcept -> ast::Ptr {
    return ast::Boolean::create(attributes, location, value);
  }

  auto getIntegerAst(Attributes attributes, Location location,
                     int value) noexcept -> ast::Ptr {
    return ast::Integer::create(attributes, location, value);
  }

  auto getParensAst(Attributes attributes, Location location,
                    ast::Ptr ast) noexcept -> ast::Ptr {
    return ast::Parens::create(attributes, location, std::move(ast));
  }

  auto getTermAst(Attributes attributes, Location location,
                  ast::Ptr ast) noexcept -> ast::Ptr {
    return ast::Affix::create(attributes, location, std::move(ast));
  }

  auto getBinopAst(Attributes attributes, Location location, Token op,
                   ast::Ptr left, ast::Ptr right) noexcept -> ast::Ptr {
    return ast::Binop::create(attributes, location, op, std::move(left),
                              std::move(right));
  }

  auto getImportAst(Attributes attributes, Location location,
                    std::string filename) noexcept -> ast::Ptr {
    return ast::Import::create(attributes, location, std::move(filename));
  }

  auto getModuleAst(Attributes attributes, Location location, Identifier name,
                    ast::Module::Expressions expressions) noexcept -> ast::Ptr {
    return ast::Module::create(attributes, location, name,
                               std::move(expressions));
  }

  auto getUnopAst(Attributes attributes, Location location, Token op,
                  ast::Ptr right) noexcept -> ast::Ptr {
    return ast::Unop::create(attributes, location, op, std::move(right));
  }

  auto getVariableAst(Attributes attributes, Location location,
                      Identifier name) noexcept -> ast::Ptr {
    return ast::Variable::create(attributes, location, name);
  }
};
} // namespace mint
