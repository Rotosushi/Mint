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
    if (at_end && more_source) {
      std::string line;
      std::getline(*in, line, '\n');
      line.push_back('\n');
      append(line);

      next();
    }
  }

  /*
    We need to call the "fill the buffer with more source routine"
    aka fill at some point during parsing.
    it cannot simply be "when the buffer is empty, and the source
    is not." because we might be parsing Ast's for the REPL, and
    in that case, we need some way to stop parsing, and return a
    single Ast for typecheck and evaluation.
    so we cannot call fill within next(), because next is called
    after we finish parsing a complete Ast, to position the parser
    to parse the next bit of input. and if we just parsed a complete
    Ast, then we don't want to ask the user for the next line of input,
    before we have typecheck and evaluate what we just parsed.
    if we did, the programmer would never be able to see the evaluation
    of what they just parsed. that is, the program would act more like
    a text editor, where you input all of you text first, before any
    evaluation. such a strategy worked when we knew a priori that
    we were processing files into executables. because we could make
    assumptions about the source.
    however now we want a single parser class which can parse for the
    REPL and for importing a file into the REPL (which is one step
    away from importing a file into the compilation process of another
    file.)
    this means all we can assume about the input is that it is a stream.
    where we may ask for more input.
    I think therefore that we need to call fill when we call peek.
    as this is explicitly when any given parse routine is asking
    to see a particular token. iff in this case the token is EOF
    then we turn to the source to ask for more input.
  */
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
  Parser(Environment *env, std::istream *in)
      : env(env), in(in), current(Token::End) {
    MINT_ASSERT(env != nullptr);
    MINT_ASSERT(in != nullptr);
  }

  [[nodiscard]] auto extractSourceLine(Location const &location) const noexcept
      -> std::string_view;

  auto parse() -> Result<Ast::Pointer> {
    /*
    if ((current == Token::End) && (!scanner.endOfInput())) {
      next(); // prime the parser with the first token
    }

    // NOT A DUPLICATE CHECK
    if (peek(Token::End)) {
      return {Error::EndOfInput, location(), ""};
    }
    */

    return parseTop();
  }
};
} // namespace mint
