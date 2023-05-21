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
#include <istream>

#include "adt/StringSet.hpp"
#include "ast/Ast.hpp"
#include "error/Error.hpp"
#include "scan/Scanner.hpp"

/*

top = let
    | affix ";"

let = "let" identifier "=" affix ";"

affix = basic binop infix-parser

binop = "+" |"-" | "*" | "/" | "%" | "!" | "&" | "|"
        "<" | "<=" | "?=" | "!=" | "=>" | ">"

basic = "nil"
      | "true"
      | "false"
      | integer
      | identifier
      | unop basic
      | "(" affix ")"

integer = [0-9]{0-9}
identifier = [a-zA-Z_]{a-zA-Z0-9_}

*/

namespace mint {
class Parser {
private:
  IdentifierSet *set;
  Scanner scanner;
  Token current;

  auto text() const noexcept { return scanner.getText(); }
  auto location() const noexcept { return scanner.getLocation(); }
  void next() noexcept { current = scanner.scan(); }

  auto expect(Token token) noexcept -> bool {
    if (current == token) {
      next();
      return true;
    }
    return false;
  }

  auto parseTop() noexcept -> expected<Ast::Pointer>;
  auto parseLet() noexcept -> expected<Ast::Pointer>;
  auto parseAffix() noexcept -> expected<Ast::Pointer>;
  auto parseInfix(Ast::Pointer left, BinopPrecedence prec) noexcept
      -> expected<Ast::Pointer>;
  auto parseBasic() noexcept -> expected<Ast::Pointer>;

public:
  Parser(IdentifierSet *set) : set(set) { MINT_ASSERT(set != nullptr); }

  auto append(std::string_view text) noexcept { scanner.append(text); }

  auto parse() -> expected<Ast::Pointer> { return parseTop(); }
};
} // namespace mint
