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

#include "adt/Result.hpp"
#include "ast/Ast.hpp"
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

term = affix ";"

let = "let" identifier (":" type)? "=" term

module = "module" identifier "{" top* "}"

affix = call (binop precedence-parser)?

call = basic ("(" (affix ("," affix)*)? ")")?

binop = "+" | "-" | "*" | "/" | "%" | "!" | "&" | "|"
        "<" | "<=" | "==" | "!=" | "=>" | ">"

basic = literal
      | integer
      | identifier
      | unop basic
      | "(" affix ")"

literal = "nil"
        | "true"
        | "false"
        | "\" (argument-list)? ("->" type)? "=>" affix
  #TODO | "\" (argument-list)? ("->" type)? block

argument-list = argument ("," argument)*

argument = identifier ":" type

type = "Nil"
     | "Boolean"
     | "Integer"
     | "\" (type ("," type)*)? "->" type

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

public:
  Parser(Environment *env, std::istream *in) noexcept;

  void setIstream(std::istream *in) noexcept;

  [[nodiscard]] auto extractSourceLine(Location const &location) const noexcept
      -> std::string_view;

  void printErrorWithSource(std::ostream &out,
                            Error const &error) const noexcept;

  auto endOfInput() const noexcept -> bool;
  auto parse() -> Result<ast::Ptr> { return parseTop(); }

private:
  auto text() const noexcept -> std::string_view;
  auto location() const noexcept -> Location;

  void next() noexcept;
  void append(std::string_view text) noexcept;
  void fill() noexcept;

  // NOTE: we call fill before we
  // check the state of the current token.
  // to ensure that we have all of the available
  // source before we check the state.
  // otherwise the end of the buffer could
  // be reached and parsing stopped when the
  // term itself is merely separated accross lines.
  auto peek(Token token) noexcept -> bool;
  auto expect(Token token) noexcept -> bool;
  auto predictsDeclaration(Token token) noexcept -> bool;

  // we just encountered a syntax error,
  // so we want to walk the parser past the
  // line of source text which produced the
  // error.
  // so advance the scanner until we see ';'
  // or the End of the buffer.
  void recover() noexcept;
  auto handle_error(Error::Kind kind) noexcept -> Error;
  auto handle_error(Error::Kind kind, Location location,
                    std::string_view message) noexcept -> Error;

  auto parseTop() noexcept -> Result<ast::Ptr>;
  auto parseDeclaration(bool is_public) noexcept -> Result<ast::Ptr>;
  auto parseModule(bool is_public) noexcept -> Result<ast::Ptr>;
  auto parseLet(bool is_public) noexcept -> Result<ast::Ptr>;
  auto parseImport() noexcept -> Result<ast::Ptr>;
  auto parseTerm() noexcept -> Result<ast::Ptr>;
  auto parseAffix() noexcept -> Result<ast::Ptr>;
  auto parseCall() noexcept -> Result<ast::Ptr>;
  auto precedenceParser(ast::Ptr left, BinopPrecedence prec) noexcept
      -> Result<ast::Ptr>;
  auto parseBasic() noexcept -> Result<ast::Ptr>;
  auto parseNil() noexcept -> Result<ast::Ptr>;
  auto parseTrue() noexcept -> Result<ast::Ptr>;
  auto parseFalse() noexcept -> Result<ast::Ptr>;
  auto parseInteger() noexcept -> Result<ast::Ptr>;
  auto parseVariable() noexcept -> Result<ast::Ptr>;
  auto parseUnop() noexcept -> Result<ast::Ptr>;
  auto parseParens() noexcept -> Result<ast::Ptr>;
  auto parseLambda() noexcept -> Result<ast::Ptr>;
  auto parseType() noexcept -> Result<type::Ptr>;
  auto parseNilType() noexcept -> Result<type::Ptr>;
  auto parseBooleanType() noexcept -> Result<type::Ptr>;
  auto parseIntegerType() noexcept -> Result<type::Ptr>;
  auto parseFunctionType() noexcept -> Result<type::Ptr>;

public:
};
} // namespace mint
