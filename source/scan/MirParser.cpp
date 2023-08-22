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
#include "scan/MirParser.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"
#include "utility/NumbersRoundTrip.hpp"
#include "utility/Stringify.hpp"

namespace mint {
Result<ir::detail::Parameter> MirParser::parseTop(ir::Mir &mir) {
  fill();
  if (endOfInput()) {
    return {Error::Kind::EndOfInput};
  }

  if (peek(Token::Public) || peek(Token::Private) || peek(Token::Let)) {
    return parseLet(mir);
  }

  if (peek(Token::Module)) {
    return parseModule(mir);
  }

  if (peek(Token::Import)) {
    return parseImport(mir);
  }

  return parseTerm(mir);
}

Result<ir::detail::Parameter> MirParser::parseModule(ir::Mir &mir) {
  // auto lhs_loc = location();

  if (!expect(Token::Module)) {
    return recover(Error::Kind::ExpectedKeywordModule);
  }

  if (!peek(Token::Identifier)) {
    return recover(Error::Kind::ExpectedIdentifier);
  }

  auto id = env->getIdentifier(text());
  next();

  if (!expect(Token::BeginBrace)) {
    return recover(Error::Kind::ExpectedBeginBrace);
  }

  ir::Module::Expressions expressions;
  while (!expect(Token::EndBrace)) {
    ir::Mir expression;
    auto result = parseTop(expression);
    if (!result) {
      return result.error();
    }

    expressions.emplace_back(std::move(expression));
  }

  // auto rhs_loc = location();
  // Location module_loc{lhs_loc, rhs_loc};
  return mir.emplaceModule(id, std::move(expressions));
}

Result<ir::detail::Parameter> MirParser::parseImport(ir::Mir &mir) {
  // auto lhs_loc = location();

  if (!expect(Token::Import)) {
    return recover(Error::Kind::ExpectedKeywordImport);
  }

  if (!peek(Token::Text)) {
    return recover(Error::Kind::ExpectedText);
  }

  auto filename = stringify(text());
  next();

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  // auto rhs_loc = location();
  // Location import_loc{lhs_loc, rhs_loc};
  return mir.emplaceImport(filename);
}

Result<ir::detail::Parameter> MirParser::parseLet(ir::Mir &mir) {
  // Attributes attributes;

  if (expect(Token::Public)) {
    // attributes.isPublic(true);
  } else if (expect(Token::Private)) {
    // attributes.isPublic(false);
  }

  if (!expect(Token::Let)) {
    return recover(Error::Kind::ExpectedKeywordLet);
  }

  if (!peek(Token::Identifier)) {
    return recover(Error::Kind::ExpectedIdentifier);
  }

  auto id = env->getIdentifier(text());
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

  auto term = parseTerm(mir);
  if (!term) {
    return term;
  }

  return mir.emplaceLet(id, annotation, term.value());
}

Result<ir::detail::Parameter> MirParser::parseTerm(ir::Mir &mir) {

  auto affix = parseAffix(mir);
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  // #TODO: Technically speaking, Term is an affix with a
  // following semicolon, so why is the Instruction representing
  // such called Affix and not Term?
  return mir.emplaceAffix(affix.value());
}

Result<ir::detail::Parameter> MirParser::parseAffix(ir::Mir &mir) {
  auto call = parseCall(mir);
  if (!call) {
    return call;
  }

  if (isBinop(m_current_token)) {
    return parseBinop(mir, call.value(), {0});
  }

  return call;
}

Result<ir::detail::Parameter> MirParser::parseCall(ir::Mir &mir) {
  auto basic = parseBasic(mir);
  if (!basic) {
    return basic;
  }

  if (!expect(Token::BeginParen)) {
    return basic;
  }

  ir::Call::Arguments arguments;

  if (!peek(Token::EndParen)) {
    do {
      auto result = parseAffix(mir);
      if (!result) {
        return result;
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  if (!expect(Token::EndParen)) {
    return recover(Error::Kind::ExpectedEndParen);
  }

  return mir.emplaceCall(basic.value(), std::move(arguments));
}

Result<ir::detail::Parameter> MirParser::parseBinop(ir::Mir &mir,
                                                    ir::detail::Parameter left,
                                                    BinopPrecedence p) {
  Result<ir::detail::Parameter> result = left;
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

    auto right = parseCall(mir);
    if (!right) {
      return right;
    }

    while (predictsHigherPrecedenceOrRightAssociativeBinop()) {
      auto temp = parseBinop(mir, result.value(), new_prec());
      if (!temp) {
        return temp;
      }

      result = temp;
    }

    result = mir.emplaceBinop(op, result.value(), right.value());
  }

  return result;
}

Result<ir::detail::Parameter> MirParser::parseBasic(ir::Mir &mir) {
  fill();

  switch (m_current_token) {
  case Token::Nil:
    return parseNil(mir);

  case Token::True:
    return parseTrue(mir);

  case Token::False:
    return parseFalse(mir);

  case Token::Integer:
    return parseInteger(mir);

  case Token::Identifier:
    return parseVariable(mir);

  case Token::Not:
  case Token::Minus:
    return parseUnop(mir);

  case Token::BeginParen:
    return parseParens(mir);

  case Token::BSlash:
    return parseLambda(mir);

  default:
    return recover(Error::Kind::ExpectedBasic);
  }
}

Result<ir::detail::Parameter>
MirParser::parseNil([[maybe_unused]] ir::Mir &mir) {
  next();
  return {};
}

Result<ir::detail::Parameter>
MirParser::parseTrue([[maybe_unused]] ir::Mir &mir) {
  next();
  return {true};
}

Result<ir::detail::Parameter>
MirParser::parseFalse([[maybe_unused]] ir::Mir &mir) {
  next();
  return {false};
}

Result<ir::detail::Parameter>
MirParser::parseInteger([[maybe_unused]] ir::Mir &mir) {
  auto value = fromString<int>(text());
  next();
  return {value};
}

Result<ir::detail::Parameter>
MirParser::parseVariable([[maybe_unused]] ir::Mir &mir) {
  auto name = env->getIdentifier(text());
  next();
  return {name};
}

Result<ir::detail::Parameter> MirParser::parseUnop(ir::Mir &mir) {
  auto op = m_current_token;
  next();

  auto right = parseBasic(mir);
  if (!right) {
    return right;
  }

  return mir.emplaceUnop(op, right.value());
}

Result<ir::detail::Parameter> MirParser::parseParens(ir::Mir &mir) {
  if (!expect(Token::BeginParen)) {
    return recover(Error::Kind::ExpectedBeginParen);
  }

  auto affix = parseAffix(mir);
  if (!affix) {
    return affix;
  }

  if (!expect(Token::EndParen)) {
    return recover(Error::Kind::ExpectedEndParen);
  }

  return mir.emplaceParens(affix.value());
}

Result<ir::detail::Parameter> MirParser::parseLambda(ir::Mir &mir) {
  if (!expect(Token::BSlash)) {
    return recover(Error::Kind::ExpectedBackSlash);
  }

  auto parseArgument = [&]() -> Result<FormalArgument> {
    if (!peek(Token::Identifier)) {
      return recover(Error::Kind::ExpectedIdentifier);
    }

    auto name = env->getIdentifier(text());
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
        return result;
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  std::optional<type::Ptr> annotation;
  if (expect(Token::RArrow)) {
    auto result = parseType();
    if (!result) {
      return result;
    }

    annotation = result.value();
  }

  if (!expect(Token::EqRArrow)) {
    return recover(Error::Kind::ExpectedEqualsRightArrow);
  }

  auto result = parseAffix(mir);
  if (!result) {
    return result;
  }

  return mir.emplaceLambda(std::move(arguments), annotation, result.value());
}

Result<type::Ptr> MirParser::parseType() {
  fill();

  switch (m_current_token) {
  case Token::NilType:
    return parseNilType();

  case Token::BooleanType:
    return parseBooleanType();

  case Token::IntegerType:
    return parseIntegerType();

  case Token::BSlash:
    return parseFunctionType();

  default:
    return recover(Error::Kind::ExpectedType);
  }
}
Result<type::Ptr> MirParser::parseNilType() {
  next();
  return env->getNilType();
}

Result<type::Ptr> MirParser::parseBooleanType() {
  next();
  return env->getBooleanType();
}

Result<type::Ptr> MirParser::parseIntegerType() {
  next();
  return env->getIntegerType();
}

Result<type::Ptr> MirParser::parseFunctionType() {
  next();
  type::Function::Arguments arguments;

  if (!peek(Token::RArrow)) {
    do {
      auto result = parseType();
      if (!result) {
        return result;
      }

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
  }

  if (!expect(Token::RArrow)) {
    return recover(Error::Kind::ExpectedRightArrow);
  }

  auto result = parseType();
  if (!result) {
    return result;
  }

  return env->getFunctionType(result.value(), std::move(arguments));
}
} // namespace mint
