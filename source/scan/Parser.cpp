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
top = visibility? declaration
    | term
*/
auto Parser::parseTop() noexcept -> Result<Ast::Pointer> {
  if (peek(Token::Public)) {
    next();
    return parseDeclaration(/* is_public = */ true);
  } else if (peek(Token::Private)) {
    next();
    return parseDeclaration(/* is_public = */ false);
  } else if (predictsDeclaration(current)) {
    return parseDeclaration(/* is_public = */ false);
  } else if (peek(Token::Import)) {
    return parseImport();
  } else {
    return parseTerm();
  }
}

/*
  declaration = let
              | module
*/
auto Parser::parseDeclaration(bool is_public) noexcept -> Result<Ast::Pointer> {
  if (peek(Token::Let)) {
    return parseLet(is_public);
  } else if (peek(Token::Module)) {
    return parseModule(is_public);
  } else {
    return {Error::ExpectedADeclaration, location(), text()};
  }
}

/*
  let = "let" identifier "=" term
*/
auto Parser::parseLet(bool is_public) noexcept -> Result<Ast::Pointer> {
  Attributes attributes = default_attributes;
  attributes.isPublic(is_public);
  auto left_loc = location();
  MINT_ASSERT(peek(Token::Let));
  next(); // eat 'let'

  if (!peek(Token::Identifier)) {
    return handle_error(Error::ExpectedAnIdentifier, location(), text());
  }

  auto id = env->getIdentifier(text());
  next(); // eat identifier

  if (!expect(Token::Equal)) {
    return handle_error(Error::ExpectedAnEquals, location(), text());
  }

  auto affix = parseTerm();
  if (!affix) {
    return affix;
  }

  auto right_loc = location();
  Location let_loc = {left_loc, right_loc};
  return {env->getLetAst(attributes, let_loc, id, affix.value())};
}

/*
  module = "module" identifier "{" top* "}"
*/
auto Parser::parseModule(bool is_public) noexcept -> Result<Ast::Pointer> {
  Attributes attributes = default_attributes;
  attributes.isPublic(is_public);
  auto left_loc = location();
  /* "module" identifier "{" */
  MINT_ASSERT(peek(Token::Module));
  next(); // eat 'module'

  if (!peek(Token::Identifier)) {
    return handle_error(Error::ExpectedAnIdentifier, location(), text());
  }

  auto id = env->getIdentifier(text());
  next();

  if (!expect(Token::BeginBrace)) {
    return handle_error(Error::ExpectedABeginBrace, location(), text());
  }

  std::vector<Ast::Pointer> expressions;
  /* top* '}' */
  while (!expect(Token::EndBrace)) {
    auto expr = parseTop();
    if (!expr) {
      return expr;
    }
    expressions.emplace_back(std::move(expr.value()));
  }

  auto right_loc = location();
  Location module_loc = {left_loc, right_loc};
  return {
      env->getModuleAst(attributes, module_loc, id, std::move(expressions))};
}

/*
  import = "import" identifier ("from" identifier)? ";"
*/
auto Parser::parseImport() noexcept -> Result<Ast::Pointer> {
  auto left_loc = location();
  MINT_ASSERT(peek(Token::Import));
  next(); // eat "import"


  auto right_loc = location();
  Location import_loc = {left_loc, right_loc};

}

/* term = affix? ";" */
auto Parser::parseTerm() noexcept -> Result<Ast::Pointer> {
  auto left_loc = location();

  std::optional<Ast::Pointer> affix;
  if (!expect(Token::Semicolon)) {
    auto result = parseAffix();
    if (!result) {
      return result;
    }
    affix = result.value();

    if (!expect(Token::Semicolon)) {
      return handle_error(Error::ExpectedASemicolon, location(), text());
    }
  }

  auto right_loc = location();
  Location term_loc = {left_loc, right_loc};
  return env->getTermAst(default_attributes, term_loc, affix);
}

auto Parser::parseAffix() noexcept -> Result<Ast::Pointer> {
  auto basic = parseBasic();
  if (!basic) {
    return basic;
  }

  if (isBinop(current)) {
    return precedenceParser(std::move(basic.value()), 0);
  }

  return basic;
}

// #TODO: I'm fairly sure that location tracking in
// precedence parsing has a bug in it.
// ... a + b ...
// has location information such that we will highlight
// ... a + b ...
// ...---^^^--...
// instead of the (probably) expected
// ... a + b ...
// ...-^^^^^-...

auto Parser::precedenceParser(Ast::Pointer left, BinopPrecedence prec) noexcept
    -> Result<Ast::Pointer> {
  Result<Ast::Pointer> result = left;
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

    if ((associativity(op) == BinopAssociativity::Right) &&
        (precedence(current) == precedence(op)))
      return true;

    return false;
  };

  auto new_prec = [&]() -> BinopPrecedence {
    if (precedence(op) > precedence(current))
      return precedence(op) + 1;
    else
      return precedence(op);
  };

  while (predicts_binop()) {
    op = current;
    op_loc = location();

    next(); // eat 'op'

    auto right = parseBasic();
    if (!right)
      return right;

    while (predictsHigherPrecedenceOrRightAssociativeBinop()) {
      auto temp = precedenceParser(right.value(), new_prec());
      if (!temp)
        return temp;

      right = temp;
    }

    auto rhs_loc = ast_location(right.value());
    Location binop_loc = {op_loc, rhs_loc};
    Ast::Pointer lhs = result.value();
    Ast::Pointer rhs = right.value();
    result = env->getBinopAst(default_attributes, binop_loc, op, lhs, rhs);
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
auto Parser::parseBasic() noexcept -> Result<Ast::Pointer> {
  fill();

  switch (current) {
  case Token::Nil: {
    auto loc = location();
    next();
    return env->getNilAst(default_attributes, loc);
    break;
  }

  case Token::True: {
    auto loc = location();
    next();
    return env->getBooleanAst(default_attributes, loc, true);
    break;
  }

  case Token::False: {
    auto loc = location();
    next();
    return env->getBooleanAst(default_attributes, loc, false);
    break;
  }

  case Token::Integer: {
    int value = StringToNumber<int>(text());
    auto loc = location();
    next();

    return env->getIntegerAst(default_attributes, loc, value);
    break;
  }

  case Token::Identifier: {
    auto name = env->getIdentifier(text());
    auto loc = location();
    next(); // eat 'id'

    return env->getVariableAst(default_attributes, loc, name);
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

    return env->getUnopAst(default_attributes, unop_loc, op, right.value());
    break;
  }

  case Token::BeginParen: {
    next(); // eat '('

    auto affix = parseAffix();
    if (!affix)
      return affix;

    if (!expect(Token::EndParen)) {
      return handle_error(Error::ExpectedAClosingParen, location(), text());
    }

    return env->getParensAst(default_attributes, ast_location(affix.value()),
                             affix.value());
    break;
  }

  default:
    return handle_error(Error::ExpectedABasicTerm, location(), text());
    break;
  }
}
} // namespace mint
