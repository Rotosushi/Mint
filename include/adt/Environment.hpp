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

#include "adt/BinopTable.hpp"
#include "adt/DirectorySearch.hpp"
#include "adt/Identifier.hpp"
#include "adt/Scope.hpp"
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"

#include "scan/Parser.hpp"

namespace mint {
class Environment {
  DirectorySearcher directory_searcher;
  IdentifierSet id_interner;
  TypeInterner type_interner;
  BinopTable binop_table;
  UnopTable unop_table;
  // #NOTE: since we only have std::weak_ptrs
  // back up the tree of scopes, in order to
  // prevent references, we must hold a shared_ptr
  // to the top of the tree, global_scope. such that
  // when we traverse to a lower scope, all of the
  // above scopes stay alive.
  std::shared_ptr<Scope> global_scope;
  std::shared_ptr<Scope> local_scope;
  Parser parser;

  std::istream *in;
  std::ostream *out;
  std::ostream *errout;

public:
  Environment(std::istream *in = &std::cin, std::ostream *out = &std::cout,
              std::ostream *errout = &std::cerr) noexcept
      : global_scope(Scope::createGlobalScope()), local_scope(global_scope),
        parser(this, in), in(in), out(out), errout(errout) {
    MINT_ASSERT(in != nullptr);
    MINT_ASSERT(out != nullptr);
    MINT_ASSERT(errout != nullptr);

    InitializeBuiltinBinops(this);
    InitializeBuiltinUnops(this);
  }

  auto repl() noexcept -> int;

  void printErrorWithSource(Error const &error) const noexcept {
    auto optional_location = error.getLocation();
    std::string_view bad_source;
    if (optional_location.has_value())
      bad_source = parser.extractSourceLine(optional_location.value());

    error.print(*errout, bad_source);
  }

  void printErrorWithSource(Error const &error,
                            Parser const &parser) const noexcept {
    auto optional_location = error.getLocation();
    std::string_view bad_source;
    if (optional_location.has_value())
      bad_source = parser.extractSourceLine(optional_location.value());

    error.print(*errout, bad_source);
  }

  auto getString(std::string_view string) noexcept -> std::string_view {
    auto cursor = std::next(string.begin());
    auto end = std::prev(string.end());

    // #TODO: replace escape sequences with character literals here.

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
    return id_interner.emplace(name);
  }

  /*
    intended to be called when processing an Ast::Function,
    such that lookup and binding occurs within the local
    scope of the function.
    note the lack of a scope name, these anonymous scopes
    are only alive as long as they are the local_scope.

    this behavior is okay for functions, but not for
    modules, as a modules public names need to be available
    for qualified name lookup
  */
  void pushScope() noexcept {
    local_scope = Scope::createScope({}, local_scope);
  }

  /*
    intended to be called when processing an Ast::Module,
    such that lookup and binding occurs within a
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

  void unbindScope(Identifier name) noexcept { local_scope->unbindScope(name); }

  void popScope() noexcept {
    if (local_scope->isGlobal()) {
      return; // cannot traverse past global scope
    }

    local_scope = local_scope->getPrevScope();
  }

  auto bindName(Identifier name, Attributes attributes, Type::Pointer type,
                Ast::Ptr value) noexcept {
    return local_scope->bindName(name, attributes, type, value);
  }

  auto lookup(Identifier name) { return local_scope->lookup(name); }

  auto createBinop(Token op) { return binop_table.emplace(op); }
  auto lookupBinop(Token op) { return binop_table.lookup(op); }

  auto createUnop(Token op) { return unop_table.emplace(op); }
  auto lookupUnop(Token op) { return unop_table.lookup(op); }

  auto getBooleanType() noexcept { return type_interner.getBooleanType(); }
  auto getIntegerType() noexcept { return type_interner.getIntegerType(); }
  auto getNilType() noexcept { return type_interner.getNilType(); }

  auto getTypeAst(Attributes attributes, Location location,
                  mint::Type::Pointer type) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Type>, attributes,
                                 location, type);
  }

  // eventually all ast's are going to be constructed via custom
  // allocators. such that an environment can be allocated entirely
  // within a custom allocator. (Types, Asts, Binops, and Unops
  // are all considered "part of" a given Environment.)
  auto getModuleAst(Attributes attributes, Location location, Identifier name,
                    std::vector<Ast::Ptr> expressions) noexcept {
    auto alloc = new Ast(std::in_place_type<Ast::Module>, attributes, location,
                         name, std::move(expressions));
    return std::shared_ptr<Ast>(alloc);
  }

  auto getLetAst(Attributes attributes, Location location, Identifier name,
                 Ast::Ptr term) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Let>, attributes,
                                 location, name, term);
  }

  auto getImportAst(Attributes attributes, Location location,
                    std::string_view file) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Import>, attributes,
                                 location, file);
  }

  auto getBinopAst(Attributes attributes, Location location, Token op,
                   Ast::Ptr left, Ast::Ptr right) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Binop>, attributes,
                                 location, op, left, right);
  }

  auto getUnopAst(Attributes attributes, Location location, Token op,
                  Ast::Ptr right) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Unop>, attributes,
                                 location, op, right);
  }

  auto getTermAst(Attributes attributes, Location location,
                  std::optional<Ast::Ptr> ast) noexcept {
    auto alloc =
        new Ast(std::in_place_type<Ast::Term>, attributes, location, ast);
    return std::shared_ptr<Ast>(alloc);
  }

  auto getParensAst(Attributes attributes, Location location,
                    Ast::Ptr ast) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Parens>, attributes,
                                 location, ast);
  }

  auto getVariableAst(Attributes attributes, Location location,
                      Identifier name) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Variable>, attributes,
                                 location, name);
  }

  auto getBooleanAst(Attributes attributes, Location location,
                     bool value) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Boolean>,
                                 attributes, location, value);
  }

  auto getIntegerAst(Attributes attributes, Location location,
                     int value) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Integer>,
                                 attributes, location, value);
  }

  auto getNilAst(Attributes attributes, Location location) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Nil>,
                                 attributes, location);
  }
};
} // namespace mint
