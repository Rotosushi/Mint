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

#include "utility/NumbersRoundTrip.hpp"

#include "adt/Environment.hpp"

namespace mint {
auto Parser::extractSourceLine(Location const &location) const noexcept
    -> std::string_view {
  auto view = scanner.view();
  auto cursor = view.begin();
  auto end = view.end();
  std::size_t lines_seen = 1;

  while ((lines_seen < location.fline) && (cursor != end)) {
    if (*cursor == '\n') {
      lines_seen++;
    }
    cursor++;
  }

  if (cursor != end) {
    auto eol = cursor;

    while (eol != end && *eol != '\n') {
      eol++;
    }
    return {cursor, eol};
  }
  return {};
}

/*
  term = let
    | affix ";"
*/
auto Parser::parseTop() noexcept -> ParseResult {
  if (current == Token::Let)
    return parseLet();
  else {
    auto affix = parseAffix();
    if (!affix) {
      return affix;
    }

    if (!expect(Token::Semicolon)) {
      return ParseResult{std::unexpect, Error::ExpectedASemicolon, location(),
                         text()};
    }

    return affix;
  }
}

/*
  let = "let" identifier "=" affix ";"
*/
auto Parser::parseLet() noexcept -> ParseResult {
  auto left_loc = location();
  next(); // eat 'let'

  if (current != Token::Identifier) {
    return ParseResult{std::unexpect, Error::ExpectedAnIdentifier, location(),
                       text()};
  }

  auto id = text();

  next(); // eat identifier

  if (!expect(Token::Equal)) {
    return ParseResult{std::unexpect, Error::ExpectedAnEquals, location(),
                       text()};
  }

  auto affix = parseAffix();
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return ParseResult{std::unexpect, Error::ExpectedASemicolon, location(),
                       text()};
  }

  auto right_loc = location();
  Location let_loc = {left_loc, right_loc};
  return {env->getLetAst(let_loc, id, affix.value())};
}

auto Parser::parseAffix() noexcept -> ParseResult {
  auto basic = parseBasic();
  if (!basic) {
    return basic;
  }

  if (isBinop(current)) {
    return parseInfix(std::move(basic.value()), 0);
  }

  return basic;
}

// #TODO: I'm fairly sure that location tracking in binop
// expressions has a bug in it.
// ... a + b ...
// has location information such that we will highlight
// ... a + b ...
// ...---^^^--...
// instead of the (probably) expected
// ... a + b ...
// ...-^^^^^-...
auto Parser::parseInfix(Ast *left, BinopPrecedence prec) noexcept
    -> ParseResult {
  ParseResult result = left;
  Location op_loc;
  Token op{Token::Error};

  auto predicts_binop = [&]() -> bool {
    if (!isBinop(current))
      return false;

    return precedence(current) >= prec;
  };

  auto predictsHigherPrecedenceOrRightAssociativeBinop = [&]() -> bool {
    if (!isBinop(current))
      return false;

    if (precedence(current) > precedence(op))
      return true;

    if ((associativity(current) == BinopAssociativity::Right) &&
        (precedence(current) == precedence(op)))
      return true;

    return false;
  };

  while (predicts_binop()) {
    op = current;
    op_loc = location();

    next(); // eat 'op'

    auto right = parseBasic();
    if (!right)
      return right;

    while (predictsHigherPrecedenceOrRightAssociativeBinop()) {
      auto temp = parseInfix(right.value(), precedence(op));
      if (!temp)
        return temp;

      result = temp;
    }

    auto rhs_loc = location();
    Location binop_loc = {op_loc, rhs_loc};
    result = env->getBinopAst(binop_loc, op, result.value(), right.value());
  }

  return result;
}

/*
basic = "nil"
      | "true"
      | "false"
      | integer
      | identifier
      | unop basic
      | "(" affix ")"
*/
auto Parser::parseBasic() noexcept -> ParseResult {
  switch (current) {
  case Token::Nil: {
    auto loc = location();
    next();
    return env->getNilAst(loc);
    break;
  }

  case Token::True: {
    auto loc = location();
    next();
    return env->getBooleanAst(loc, true);
    break;
  }

  case Token::False: {
    auto loc = location();
    next();
    return env->getBooleanAst(loc, false);
    break;
  }

  case Token::Integer: {
    int value = StringToNumber<int>(text());
    auto loc = location();
    next();

    return env->getIntegerAst(loc, value);
    break;
  }

  case Token::Identifier: {
    auto name = env->getIdentifier(text());
    auto loc = location();
    next(); // eat 'id'

    return env->getVariableAst(loc, name);
    break;
  }

  case Token::Not:
  case Token::Minus: {
    auto lhs_loc = location();
    auto op = current;
    next(); // eat unop

    auto right = parseBasic();
    if (!right)
      return right;

    auto rhs_loc = location();
    Location unop_loc{lhs_loc, rhs_loc};

    return env->getUnopAst(unop_loc, op, right.value());
  }

  case Token::LParen: {
    next(); // eat '('

    auto affix = parseAffix();
    if (!affix)
      return affix;

    if (!expect(Token::RParen)) {
      return ParseResult{std::unexpect, Error::ExpectedAClosingParen,
                         location(), text()};
    }

    return env->getParensAst(affix.value()->location, affix.value());
    break;
  }

  default:
    return ParseResult{std::unexpect, Error::ExpectedABasicTerm, location(),
                       text()};
    break;
  }
}
} // namespace mint
