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
#include "adt/Environment.hpp"
#include "utility/NumbersRoundTrip.hpp"
#include "utility/Stringify.hpp"

namespace mint {
Parser::Parser(Environment &env) noexcept
    : m_env(&env), m_sources(InputStream{env.inputStream()}),
      m_lexer(m_sources.peek()), m_current_token(Token::End) {}

Result<ast::Ptr> Parser::parseTop() {
  fill();
  if (endOfInput()) {
    return {Error::Kind::EndOfInput};
  }

  if (peek(Token::Module)) {
    return parseModule();
  }

  if (peek(Token::Import)) {
    return parseImport();
  }

  return parseTerm();
}

Result<ast::Ptr> Parser::parseModule() {
  auto lhs_loc = location();

  if (!expect(Token::Module)) {
    return recover(Error::Kind::ExpectedKeywordModule);
  }

  if (!peek(Token::Identifier)) {
    return recover(Error::Kind::ExpectedIdentifier);
  }

  auto id = m_env->getIdentifier(text());
  next();

  if (!expect(Token::BeginBrace)) {
    return recover(Error::Kind::ExpectedBeginBrace);
  }

  ast::Module::Expressions expressions;
  while (!expect(Token::EndBrace)) {
    auto result = parseTop();
    if (!result) {
      return result.error();
    }

    expressions.emplace_back(std::move(result.value()));
  }

  auto rhs_loc = location();
  Location module_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Module>(source(module_loc), id,
                                  std::move(expressions));
}

Result<ast::Ptr> Parser::parseImport() {
  auto lhs_loc = location();

  if (!expect(Token::Import)) {
    return recover(Error::Kind::ExpectedKeywordImport);
  }

  if (!peek(Token::Text)) {
    return recover(Error::Kind::ExpectedText);
  }

  auto filename = m_env->internString(stringify(text()));
  next();

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  auto rhs_loc = location();
  Location import_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Import>(source(import_loc), filename);
}

Result<ast::Ptr> Parser::parseTerm() {
  if (peek(Token::Public) || peek(Token::Private)) {
    return parseVisibility();
  }

  if (peek(Token::Let) || peek(Token::Fn)) {
    // #TODO: allow the default visibility to be set by the source code.
    return parseDefinition(false);
  }

  auto affix = parseAffix();
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  return affix;
}

Result<ast::Ptr> Parser::parseVisibility() {
  if (expect(Token::Public)) {
    return parseDefinition(true);
  }

  if (expect(Token::Private)) {
    return parseDefinition(false);
  }

  return recover(Error::Kind::ExpectedVisibility);
}

Result<ast::Ptr> Parser::parseDefinition(bool visibility) {
  if (peek(Token::Let)) {
    return parseLet(visibility);
  }

  if (peek(Token::Fn)) {
    return parseFunction(visibility);
  }

  return recover(Error::Kind::ExpectedDefinition);
}

Result<ast::Ptr> Parser::parseLet(bool visibility) {
  auto lhs_loc = location();
  Attributes attributes;
  attributes.isPublic(visibility);

  if (!expect(Token::Let)) {
    return recover(Error::Kind::ExpectedKeywordLet);
  }

  if (!peek(Token::Identifier)) {
    return recover(Error::Kind::ExpectedIdentifier);
  }

  auto id = m_env->getIdentifier(text());
  next();

  std::optional<type::Ptr> annotation;
  if (expect(Token::Colon)) {
    auto result = parseType();
    if (!result) {
      return result.error();
    }

    annotation = result.value();
  }

  if (!expect(Token::Equal)) {
    return recover(Error::Kind::ExpectedEquals);
  }

  auto result = parseAffix();
  if (!result) {
    return result;
  }
  auto &affix = result.value();

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  auto rhs_loc = location();
  Location let_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Let>(source(let_loc), id, attributes, annotation,
                               std::move(affix));
}

Result<ast::Ptr> Parser::parseFunction(bool visibility) {
  auto lhs_loc = location();
  Attributes attributes;
  attributes.isPublic(visibility);

  if (!expect(Token::Fn)) {
    return recover(Error::Kind::ExpectedKeywordFn);
  }

  if (!peek(Token::Identifier)) {
    return recover(Error::Kind::ExpectedIdentifier);
  }

  auto name = m_env->getIdentifier(text());
  next();

  auto parseArgument = [&]() -> Result<FormalArgument> {
    if (!peek(Token::Identifier)) {
      return Error{Error::Kind::ExpectedIdentifier, source(location()), text()};
    }

    auto arg_name = m_env->getIdentifier(text());
    next();

    if (!expect(Token::Colon)) {
      return Error{Error::Kind::ExpectedColon, source(location()), text()};
    }

    auto result = parseType();
    if (!result) {
      return result.error();
    }
    auto type = result.value();

    return {arg_name, Attributes{}, type};
  };

  auto parseArgumentList = [&]() -> Result<FormalArguments> {
    FormalArguments formal_arguments;

    if (!expect(Token::BeginParen)) {
      return Error{Error::Kind::ExpectedBeginParen, source(location()), text()};
    }

    if (!peek(Token::EndParen)) {
      do {
        auto result = parseArgument();
        if (!result) {
          return result.error();
        }
        formal_arguments.emplace_back(result.value());
      } while (expect(Token::Comma));
    }

    if (!expect(Token::EndParen)) {
      return Error{Error::Kind::ExpectedEndParen, source(location()), text()};
    }

    return formal_arguments;
  };

  auto arguments_result = parseArgumentList();
  if (!arguments_result) {
    return recover(arguments_result.error());
  }
  auto &arguments = arguments_result.value();

  std::optional<type::Ptr> annotation;
  if (expect(Token::RightArrow)) {
    auto result = parseType();
    if (!result) {
      return result.error();
    }
    annotation = result.value();
  }

  // #TODO: add support for a single expression function
  // without braces.
  ast::Function::Body body;
  if (!expect(Token::BeginBrace)) {
    return recover(Error::Kind::ExpectedBeginBrace);
  }

  while (!peek(Token::EndBrace)) {
    auto result = parseTerm();
    if (!result) {
      return result;
    }
    body.emplace_back(std::move(result.value()));
  }

  if (!expect(Token::EndBrace)) {
    return recover(Error::Kind::ExpectedEndBrace);
  }

  auto rhs_loc = location();
  Location fn_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Function>(source(fn_loc), name, attributes,
                                    std::move(arguments), annotation,
                                    std::move(body));
}

Result<ast::Ptr> Parser::parseAffix() {
  auto call = parseCall();
  if (!call) {
    return call;
  }

  if (isBinop(m_current_token)) {
    return parseBinop(call.value(), {0});
  }

  return call;
}

Result<ast::Ptr> Parser::parseCall() {
  auto lhs_loc = location();
  auto basic = parseBasic();
  if (!basic) {
    return basic;
  }

  if (!expect(Token::BeginParen)) {
    return basic;
  }

  ast::Call::Arguments arguments;
  if (!peek(Token::EndParen)) {
    do {
      auto result = parseAffix();
      if (!result) {
        return result;
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  if (!expect(Token::EndParen)) {
    return recover(Error::Kind::ExpectedEndParen);
  }

  auto rhs_loc = location();
  Location call_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Call>(source(call_loc), basic.value(),
                                std::move(arguments));
}

Result<ast::Ptr> Parser::parseBinop(ast::Ptr left, BinopPrecedence p) {
  auto lhs_loc = location();
  Result<ast::Ptr> result = left;
  Token op{Token::Error};

  auto new_prec = [&]() -> BinopPrecedence {
    if (precedence(op) > precedence(m_current_token))
      return precedence(op) + (BinopPrecedence)1U;
    else
      return precedence(op);
  };

  auto predictsBinop = [&]() -> bool {
    if (!isBinop(m_current_token)) {
      return false;
    }

    return precedence(m_current_token) >= p;
  };

  auto predictsHigherPrecedenceOrRightAssociativeBinop = [&]() -> bool {
    if (!isBinop(m_current_token))
      return false;

    if (precedence(m_current_token) > precedence(op))
      return true;

    if ((associativity(op) == BinopAssociativity::Right) &&
        (precedence(m_current_token) == precedence(op)))
      return true;

    return false;
  };

  while (predictsBinop()) {
    op = m_current_token;

    next();

    auto right = parseBasic();
    if (!right) {
      return right;
    }

    while (predictsHigherPrecedenceOrRightAssociativeBinop()) {
      auto temp = parseBinop(std::move(result.value()), new_prec());
      if (!temp) {
        return temp;
      }

      result = temp;
    }

    auto rhs_loc = location();
    Location binop_loc{lhs_loc, rhs_loc};
    result = ast::create<ast::Binop>(source(binop_loc), op, result.value(),
                                     right.value());
  }

  return result;
}

Result<ast::Ptr> Parser::parseBasic() {
  fill();

  switch (m_current_token) {
  case Token::Nil:
    return parseNil();

  case Token::True:
    return parseTrue();

  case Token::False:
    return parseFalse();

  case Token::Integer:
    return parseInteger();

  case Token::Identifier:
    return parseVariable();

  case Token::Not:
  case Token::Minus:
    return parseUnop();

  case Token::BeginParen:
    return parseParens();

  case Token::BackSlash:
    return parseLambda();

  default:
    return recover(Error::Kind::ExpectedBasic);
  }
}

Result<ast::Ptr> Parser::parseNil() {
  auto sl = source();
  next();
  return ast::create<std::monostate>(sl);
}

Result<ast::Ptr> Parser::parseTrue() {
  auto sl = source();
  next();
  return ast::create<bool>(sl, true);
}

Result<ast::Ptr> Parser::parseFalse() {
  auto sl = source();
  next();
  return ast::create<bool>(sl, false);
}

Result<ast::Ptr> Parser::parseInteger() {
  auto sl = source();
  auto value = fromString<int>(text());
  next();
  return ast::create<int>(sl, value);
}

Result<ast::Ptr> Parser::parseVariable() {
  auto sl = source();
  auto name = m_env->getIdentifier(text());
  next();
  return ast::create<Identifier>(sl, name);
}

Result<ast::Ptr> Parser::parseUnop() {
  auto lhs_loc = location();
  auto op = m_current_token;
  next();

  auto right = parseBasic();
  if (!right) {
    return right;
  }

  auto rhs_loc = location();
  Location unop_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Unop>(m_lexer.source(unop_loc), op,
                                std::move(right.value()));
}

Result<ast::Ptr> Parser::parseParens() {
  auto lhs_loc = location();
  if (!expect(Token::BeginParen)) {
    return recover(Error::Kind::ExpectedBeginParen);
  }

  auto result = parseAffix();
  if (!result) {
    return result;
  }
  auto &affix = result.value();

  if (!expect(Token::EndParen)) {
    return recover(Error::Kind::ExpectedEndParen);
  }

  auto rhs_loc = location();
  Location parens_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Parens>(source(parens_loc), std::move(affix));
}

Result<ast::Ptr> Parser::parseLambda() {
  auto lhs_loc = location();
  if (!expect(Token::BackSlash)) {
    return recover(Error::Kind::ExpectedBackSlash);
  }

  auto parseArgument = [&]() -> Result<FormalArgument> {
    if (!peek(Token::Identifier)) {
      return recover(Error::Kind::ExpectedIdentifier);
    }

    auto name = m_env->getIdentifier(text());
    next();

    if (!expect(Token::Colon)) {
      return recover(Error::Kind::ExpectedColon);
    }

    auto annotation = parseType();
    if (!annotation) {
      return annotation.error();
    }
    return {name, Attributes{}, annotation.value()};
  };

  FormalArguments arguments;
  if (peek(Token::Identifier)) {
    do {
      auto result = parseArgument();
      if (!result) {
        return result.error();
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  std::optional<type::Ptr> annotation;
  if (expect(Token::RightArrow)) {
    auto result = parseType();
    if (!result) {
      return result.error();
    }

    annotation = result.value();
  }

  if (!expect(Token::EqualsRightArrow)) {
    return recover(Error::Kind::ExpectedEqualsRightArrow);
  }

  // #TODO add support for multiple expressions surrounded by {}
  ast::Function::Body body;
  auto result = parseAffix();
  if (!result) {
    return result;
  }
  body.emplace_back(std::move(result.value()));

  auto rhs_loc = location();
  Location lambda_loc{lhs_loc, rhs_loc};
  return ast::create<ast::Lambda>(source(lambda_loc), Attributes{},
                                  std::move(arguments), annotation,
                                  std::move(body));
}

Result<type::Ptr> Parser::parseType() {
  fill();

  switch (m_current_token) {
  case Token::NilType:
    return parseNilType();

  case Token::BooleanType:
    return parseBooleanType();

  case Token::IntegerType:
    return parseIntegerType();

  case Token::BackSlash:
    return parseFunctionType();

  default:
    return recover(Error::Kind::ExpectedType);
  }
}
Result<type::Ptr> Parser::parseNilType() {
  next();
  return m_env->getNilType();
}

Result<type::Ptr> Parser::parseBooleanType() {
  next();
  return m_env->getBooleanType();
}

Result<type::Ptr> Parser::parseIntegerType() {
  next();
  return m_env->getIntegerType();
}

Result<type::Ptr> Parser::parseFunctionType() {
  next();
  type::Function::Arguments arguments;

  if (!peek(Token::RightArrow)) {
    do {
      auto result = parseType();
      if (!result) {
        return result;
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  if (!expect(Token::RightArrow)) {
    return recover(Error::Kind::ExpectedRightArrow);
  }

  auto result = parseType();
  if (!result) {
    return result;
  }

  return m_env->getFunctionType(result.value(), std::move(arguments));
}
} // namespace mint
