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
#include <bitset>

#include "adt/Attributes.hpp"
#include "adt/Result.hpp"
#include "adt/SourceBufferList.hpp"
#include "ast/Ast.hpp"
#include "scan/Lexer.hpp"

namespace mint {
class Environment;

/*
top = module
    | import
    | term

module = "module" identifier module-block

module-block = "{" term* "}"

import = "import" string-literal ";"

term = let
     | function
     | affix ";"

let = visibility? "let" identifier (":" type)? "=" affix ";"

function = visibility? "fn" identifier "(" formal-arguments? ")" ("->" type)?
local-block

formal-arguments =  (formal-argument ("," formal-argument)*)?

formal-argument = identifier ":" type

local-block "{" term* "}"

visibility = "public"
           | "private"

affix = call (binop precedence-parser)?

call = basic (actual-argument-list)?

actual-argument-list = "(" (affix ("," affix)*)? ")"

binop = "+" | "-" | "*" | "/" | "%" | "&" | "|"
        "<" | "<=" | "==" | "!=" | "=>" | ">"

unop = "-" | "!"


basic = literal
      | identifier
      | unop basic
      | "(" affix ")"

literal = "nil"
        | "true"
        | "false"
        | integer

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
*/
class Parser {
public:
private:
  Environment *m_env;
  // #TODO: do we want to hold all of the data from all of the files processed
  // in memory forever? or do we want to add a path to the source location
  // and reopen and resource the bad line at the point it is needed?
  // currently all input is buffered for the lifetime of the environment
  // holding the parser. even though all identifiers are Interned, and the
  // only time we need to re-scan a source file is when extracting a line of
  // source to display an error message. I think this should be changed.
  SourceBufferList m_sources;
  Lexer m_lexer;
  Token m_current_token;
  Attributes m_attributes;

public:
  Parser(Environment &env) noexcept;

  void pushSourceFile(std::fstream &&fin) {
    m_lexer.exchangeSource(m_sources.push(std::move(fin)));
  }

  void popSourceFile() { m_lexer.exchangeSource(m_sources.pop()); }

  bool endOfInput() const noexcept {
    return m_lexer.eof() && m_lexer.endOfInput();
  }

  Result<ast::Ptr> parse() { return parseTop(); }

private:
  std::string_view text() const noexcept { return m_lexer.viewToken(); }
  Location location() const noexcept { return m_lexer.currentLocation(); }
  SourceLocation *source() const noexcept {
    return m_lexer.source(m_lexer.currentLocation());
  }
  SourceLocation *source(Location const &location) const noexcept {
    return m_lexer.source(location);
  }

  void next() noexcept { m_current_token = m_lexer.lex(); }
  bool peek(Token token) noexcept {
    fill();
    return m_current_token == token;
  }
  bool expect(Token token) noexcept {
    fill();
    if (peek(token)) {
      next();
      return true;
    }
    return false;
  }

  void fill() noexcept {
    auto at_end = m_current_token == Token::End;
    auto more_source = !m_lexer.eof();
    auto good = m_lexer.good();
    if (at_end && more_source && good) {
      m_lexer.fill();
      next();
    }
  }

  Error recover(Error::Kind kind, SourceLocation *source,
                std::string_view message) noexcept {
    while (!expect(Token::Semicolon) && !peek(Token::End))
      next();

    return {kind, source, message};
  }

  Error recover(Error::Kind kind) noexcept {
    return recover(kind, source(), "");
  }

  Error recover(Error &&error) {
    while (!expect(Token::Semicolon) && !peek(Token::End))
      next();

    return error;
  }

  Result<ast::Ptr> parseTop();
  Result<ast::Ptr> parseModule();
  Result<ast::Ptr> parseImport();
  Result<ast::Ptr> parseTerm();

  Result<ast::Ptr> parseVisibility();
  Result<ast::Ptr> parseDefinition(bool visibility);
  Result<ast::Ptr> parseLet(bool visibility);
  Result<ast::Ptr> parseFunction(bool visibility);
  Result<ast::Ptr> parseAffix();

  Result<ast::Ptr> parseCall();
  Result<ast::Ptr> parseBinop(ast::Ptr left, BinopPrecedence precedence);

  Result<ast::Ptr> parseBasic();
  Result<ast::Ptr> parseNil();
  Result<ast::Ptr> parseTrue();
  Result<ast::Ptr> parseFalse();
  Result<ast::Ptr> parseInteger();
  Result<ast::Ptr> parseVariable();
  Result<ast::Ptr> parseUnop();
  Result<ast::Ptr> parseParens();

  Result<type::Ptr> parseType();
  Result<type::Ptr> parseNilType();
  Result<type::Ptr> parseBooleanType();
  Result<type::Ptr> parseIntegerType();
  Result<type::Ptr> parseFunctionType();
};
} // namespace mint
