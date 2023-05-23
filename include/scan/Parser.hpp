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
class Environment;

class Parser {
public:
  using ParseResult = Result<Ast *>;

private:
  Environment *env;
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

  auto parseTop() noexcept -> ParseResult;
  auto parseLet() noexcept -> ParseResult;
  auto parseAffix() noexcept -> ParseResult;
  auto parseInfix() noexcept -> ParseResult;
  auto precedenceParser(Ast *left, BinopPrecedence prec) noexcept
      -> ParseResult;
  auto parseBasic() noexcept -> ParseResult;

public:
  Parser(Environment *env) : env(env), current(Token::End) {
    MINT_ASSERT(env != nullptr);
  }

  [[nodiscard]] auto extractSourceLine(Location const &location) const noexcept
      -> std::string_view;

  auto append(std::string_view text) noexcept { scanner.append(text); }

  auto parse() -> ParseResult {
    if ((current == Token::End) && (!scanner.endOfInput())) {
      next(); // prime the parser with the first token
    }
    return parseTop();
  }
};
} // namespace mint
