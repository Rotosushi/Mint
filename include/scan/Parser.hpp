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
top = visibility? declaration
    | term

term = affix? ";"

declaration = let
            | module

let = "let" identifier "=" term

module = "module" identifier "{" top* "}"

affix = basic (binop precedence-parser)?

binop = "+" |"-" | "*" | "/" | "%" | "!" | "&" | "|"
        "<" | "<=" | "?=" | "!=" | "=>" | ">"

basic = "nil"
      | "true"
      | "false"
      | integer
      | identifier
      | unop basic
      | "(" affix ")"

visibility = "public"
           | "private"

integer = [0-9]+

start      = "::"?[a-zA-Z_]
continue   = [a-zA-Z0-9_];
separator  = "::";
identifier = start continue* (separator continue+)*

*/

namespace mint {
class Environment;

/*
  #TODO: parser/scanner needs to get more input from
  the source when it is out of buffer and
  the source is not empty.
*/
class Parser {
private:
  Environment *env;
  Scanner scanner;
  Token current;
  Attributes default_attributes;

  auto text() const noexcept { return scanner.getText(); }
  auto location() const noexcept { return scanner.getLocation(); }
  void next() noexcept { current = scanner.scan(); }

  auto peek(Token token) const noexcept -> bool { return current == token; }
  auto expect(Token token) noexcept -> bool {
    if (current == token) {
      next();
      return true;
    }
    return false;
  }

  auto predictsDeclaration(Token token) const noexcept -> bool {
    switch (token) {
    case Token::Let:
    case Token::Module:
      return true;
    default:
      return false;
    }
  }

  // we just encountered a syntax error,
  // so we want to walk the parser past the
  // line of source text which produced the
  // error.
  // so advance the scanner until we see ';'
  // or the End of the buffer.
  void recover() noexcept {
    while (!peek(Token::Semicolon) && !peek(Token::End)) {
      next();
    }

    if (peek(Token::Semicolon)) {
      next();
    }
  }

  auto handle_error(Error::Kind kind, Location location,
                    std::string_view message) noexcept -> Result<Ast::Pointer> {
    recover();
    return {kind, location, message};
  }

  auto parseTop() noexcept -> Result<Ast::Pointer>;
  auto parseDeclaration(bool is_public) noexcept -> Result<Ast::Pointer>;
  auto parseModule(bool is_public) noexcept -> Result<Ast::Pointer>;
  auto parseLet(bool is_public) noexcept -> Result<Ast::Pointer>;
  auto parseTerm() noexcept -> Result<Ast::Pointer>;
  auto parseAffix() noexcept -> Result<Ast::Pointer>;
  auto precedenceParser(Ast::Pointer left, BinopPrecedence prec) noexcept
      -> Result<Ast::Pointer>;
  auto parseBasic() noexcept -> Result<Ast::Pointer>;

public:
  Parser(Environment *env) : env(env), current(Token::End) {
    MINT_ASSERT(env != nullptr);
  }

  [[nodiscard]] auto extractSourceLine(Location const &location) const noexcept
      -> std::string_view;

  auto append(std::string_view text) noexcept { scanner.append(text); }

  auto parse() -> Result<Ast::Pointer> {
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
