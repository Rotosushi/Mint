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
#include "scan/Parser.hpp"

namespace mint {

/*
  term = let
    | affix ";"
*/
auto Parser::parseTop() -> expected<Ast::Pointer> {
  if (current == Token::Let)
    return parseLet();
  else {
    auto affix = parseAffix();
    if (!affix) {
      return affix;
    }

    if (!expect(Token::Semicolon)) {
      return Error{Error::ExpectedASemicolon, location(), text()};
    }

    return affix;
  }
}

/*
  let = "let" identifier "=" affix ";"
*/
auto Parser::parseLet() -> expected<Ast::Pointer> {
  next(); // eat 'let'

  if (current != Token::Identifier) {
    return Error{Error::ExpectedAnIdentifier, location(), text()};
  }

  auto id = text();

  next(); // eat identifier

  if (!expect(Token::Equal)) {
    return Error{Error::ExpectedAnEquals, location(), text()};
  }

  auto affix = parseAffix();
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return Error{Error::ExpectedASemicolon, location(), text()};
  }

  return affix;
}

auto Parser::parseAffix() -> expected<Ast::Pointer> {
  auto basic = parseBasic();
  if (!basic) {
    return basic;
  }

  if (isBinop(current)) {
    return parseInfix(std::move(basic.value()), 0);
  }

  return basic;
}

auto Parser::parseInfix(Ast::Pointer left, BinopPrecedence prec) noexcept
    -> expected<Ast::Pointer> {}

auto Parser::parseBasic() -> expected<Ast::Pointer> {
  switch (current) {
  case Token::Nil:{
    next();
    return Ast::createNil(location());
    }

  case Token::True: {
    next();
    return Ast::createBoolean(location(), true);
  }

  case Token::False: {
    next();
    return Ast::createBoolean(location(), false);
  }

  case Token::Integer: {
    next();
    
    return Ast::createInteger(location(), );
  }

  default: 
    return Error{Error::ExpectedABasicTerm, location(), text()};
  }
}
} // namespace mint
