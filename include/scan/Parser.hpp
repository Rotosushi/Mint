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
    | term

let = "let" identifier "=" term

term = affix ";"

affix = basic [binop precedence-parser]

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

  // we just encountered a syntax error,
  // so we want to walk the parser past the
  // line of source text which produced the
  // error. we require expressions to end with ';'
  // so advance the scanner through the
  // input it has buffered until we see ';'
  // or the eof.
  void recover() noexcept {
    while ((current != Token::Semicolon) && (current != Token::End)) {
      next();
    }
  }

  auto handle_error(Error::Kind kind, Location location,
                    std::string_view message) noexcept -> Result<Ast *> {
    recover();
    return {kind, location, message};
  }

  auto parseTop() noexcept -> Result<Ast *>;
  auto parseLet() noexcept -> Result<Ast *>;
  auto parseTerm() noexcept -> Result<Ast *>;
  auto parseAffix() noexcept -> Result<Ast *>;
  auto precedenceParser(Ast *left, BinopPrecedence prec) noexcept
      -> Result<Ast *>;
  auto parseBasic() noexcept -> Result<Ast *>;

public:
  Parser(Environment *env) : env(env), current(Token::End) {
    MINT_ASSERT(env != nullptr);
  }

  [[nodiscard]] auto viewSourceLine(Location const &location) const noexcept
      -> std::string_view;

  auto append(std::string_view text) noexcept { scanner.append(text); }

  auto parse() -> Result<Ast *> {
    if ((current == Token::End) && (!scanner.endOfInput())) {
      next(); // prime the parser with the first token
    }

    // NOT A DUPLICATE CHECK
    // if, after we prime the parser, we are still at eof,
    // then don't attempt to parse
    if (current == Token::End) {
      return {Error::EndOfInput, location(), ""};
    }

    return parseTop();
  }
};
} // namespace mint
