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
// #include <stack>
// #include <vector>

#include "ast/Ast.hpp"
#include "error/Error.hpp"
#include "scan/Scanner.hpp"

/*
top = visibility? declaration
    | import
    | term

visibility = "public"
           | "private"

declaration = let
            | module

import = "import" string-literal ";"

term = affix? ";"

let = "let" identifier (":" type)? "=" term

module = "module" identifier "{" top* "}"

affix = basic (binop precedence-parser)?

binop = "+" |"-" | "*" | "/" | "%" | "!" | "&" | "|"
        "<" | "<=" | "?=" | "!=" | "=>" | ">"

basic = literal
      | integer
      | identifier
      | unop basic
      | "(" affix ")"

literal = "nil"
        | "true"
        | "false"

type = "Nil"
     | "Boolean"
     | "Integer"

integer = [0-9]+

start      = "::"?[a-zA-Z_]
continue   = [a-zA-Z0-9_];
separator  = "::";
identifier = start continue* (separator continue+)*

string-literal = "\"" [.]* "\""
// text is a string literal.
*/

namespace mint {
class Environment;

class Parser {
private:
  Environment *env;
  std::istream *in;
  Scanner scanner;
  Token current;
  Attributes default_attributes;

  auto text() const noexcept { return scanner.getText(); }
  auto location() const noexcept { return scanner.getLocation(); }
  void next() noexcept { current = scanner.scan(); }

  void append(std::string_view text) noexcept { scanner.append(text); }

  void fill() noexcept {
    auto at_end = current == Token::End;
    auto more_source = !in->eof();
    auto in_good = in->good();
    if (at_end && more_source && in_good) {
      std::string line;
      std::getline(*in, line, '\n');
      line.push_back('\n');
      append(line);

      next();
    }
  }

  // NOTE: we need to call fill before we
  // check the state of the current token.
  // to ensure that we have all of the available
  // source before we check the state.
  // otherwise the end of the buffer could
  // be reached and parsing stopped when the
  // term itself is merely separated accross lines.
  auto peek(Token token) noexcept -> bool {
    fill();
    return current == token;
  }

  auto expect(Token token) noexcept -> bool {
    fill();
    if (current == token) {
      next();
      return true;
    }
    return false;
  }

  auto predictsDeclaration(Token token) noexcept -> bool {
    fill();
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

  auto handle_error(Error::Kind kind) noexcept -> Error {
    recover();
    return {kind, location(), text()};
  }
  auto handle_error(Error::Kind kind, Location location,
                    std::string_view message) noexcept -> Error {
    recover();
    return {kind, location, message};
  }

  auto parseTop() noexcept -> Result<Ast::Ptr>;
  auto parseDeclaration(bool is_public) noexcept -> Result<Ast::Ptr>;
  auto parseModule(bool is_public) noexcept -> Result<Ast::Ptr>;
  auto parseLet(bool is_public) noexcept -> Result<Ast::Ptr>;
  auto parseImport() noexcept -> Result<Ast::Ptr>;
  auto parseTerm() noexcept -> Result<Ast::Ptr>;
  auto parseAffix() noexcept -> Result<Ast::Ptr>;
  auto precedenceParser(Ast::Ptr left, BinopPrecedence prec) noexcept
      -> Result<Ast::Ptr>;
  auto parseBasic() noexcept -> Result<Ast::Ptr>;
  auto parseType() noexcept -> Result<Type::Pointer>;

public:
  Parser(Environment *env, std::istream *in)
      : env(env), in(in), current(Token::End) {
    MINT_ASSERT(env != nullptr);
    MINT_ASSERT(in != nullptr);
  }

  auto endOfInput() const noexcept { return scanner.endOfInput() && in->eof(); }

  [[nodiscard]] auto extractSourceLine(Location const &location) const noexcept
      -> std::string_view;

  auto parse() -> Result<Ast::Ptr> { return parseTop(); }
};
} // namespace mint
