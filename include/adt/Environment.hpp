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
#include "adt/IdentifierSet.hpp"
#include "adt/Scope.hpp"
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"

#include "scan/Parser.hpp"

namespace mint {
class Environment {
  TypeInterner type_interner;
  IdentifierSet identifier_set;
  BinopTable binop_table;
  UnopTable unop_table;
  Bindings scope;
  Parser parser;

  std::istream *in;
  std::ostream *out;
  std::ostream *errout;

public:
  Environment(std::istream *in = &std::cin, std::ostream *out = &std::cout,
              std::ostream *errout = &std::cerr) noexcept
      : parser(this), in(in), out(out), errout(errout) {
    MINT_ASSERT(in != nullptr);
    MINT_ASSERT(out != nullptr);
    MINT_ASSERT(errout != nullptr);

    InitializeBuiltinBinops(this);
    InitializeBuiltinUnops(this);
  }

  void printErrorWithSource(Error const &error) const noexcept {
    auto optional_location = error.getLocation();
    std::string_view bad_source;
    if (optional_location.has_value())
      bad_source = parser.extractSourceLine(optional_location.value());

    error.print(*errout, bad_source);
  }

  auto repl() noexcept -> int;

  auto bind(Identifier name, Type::Pointer type, Ast::Pointer value) noexcept {
    return scope.bind(name, type, value);
  }

  auto lookup(Identifier name) { return scope.lookup(name); }

  auto getIdentifier(std::string_view text) noexcept {
    return identifier_set.emplace(text);
  }

  auto createBinop(Token op) { return binop_table.emplace(op); }
  auto lookupBinop(Token op) { return binop_table.lookup(op); }

  auto createUnop(Token op) { return unop_table.emplace(op); }
  auto lookupUnop(Token op) { return unop_table.lookup(op); }

  auto getBooleanType() noexcept { return type_interner.getBooleanType(); }
  auto getIntegerType() noexcept { return type_interner.getIntegerType(); }
  auto getNilType() noexcept { return type_interner.getNilType(); }

  auto getTermAst(Location location, Ast::Pointer affix) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Term>, location,
                                 affix);
  }

  auto getTypeAst(Location location, mint::Type::Pointer type) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Type>, location, type);
  }

  auto getLetAst(Location location, Identifier name,
                 Ast::Pointer term) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Let>, location, name,
                                 term);
  }

  auto getBinopAst(Location location, Token op, Ast::Pointer left,
                   Ast::Pointer right) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Binop>, location, op,
                                 left, right);
  }

  auto getUnopAst(Location location, Token op, Ast::Pointer right) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Unop>, location, op,
                                 right);
  }

  auto getParensAst(Location location, Ast::Pointer ast) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Parens>, location,
                                 ast);
  }

  auto getVariableAst(Location location, Identifier name) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Variable>, location,
                                 name);
  }

  auto getBooleanAst(Location location, bool value) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Boolean>,
                                 location, value);
  }

  auto getIntegerAst(Location location, int value) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Integer>,
                                 location, value);
  }

  auto getNilAst(Location location) noexcept {
    return std::make_shared<Ast>(std::in_place_type<Ast::Value>,
                                 std::in_place_type<Ast::Value::Nil>, location);
  }
};
} // namespace mint
