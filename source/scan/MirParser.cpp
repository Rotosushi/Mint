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
MirParser::MirParser(Environment &env) noexcept
    : m_env(&env), m_sources(InputStream{env.inputStream()}),
      m_lexer(m_sources.peek()), m_current_token(Token::End) {}

Result<ir::detail::Parameter> MirParser::parseTop(ir::Mir &mir) {
  fill();
  if (endOfInput()) {
    return {Error::Kind::EndOfInput};
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

  ir::Module::Expressions expressions;
  while (!expect(Token::EndBrace)) {
    ir::Mir expression;
    auto result = parseTop(expression);
    if (!result) {
      return result.error();
    }

    expressions.emplace_back(std::move(expression));
  }

  auto rhs_loc = location();
  Location module_loc{lhs_loc, rhs_loc};
  return mir.emplaceModule(m_lexer.source(module_loc), id,
                           std::move(expressions));
}

Result<ir::detail::Parameter> MirParser::parseImport(ir::Mir &mir) {
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
  return mir.emplaceImport(source(import_loc), filename);
}

Result<ir::detail::Parameter> MirParser::parseTerm(ir::Mir &mir) {
  if (peek(Token::Public) || peek(Token::Private)) {
    return parseVisibility(mir);
  }

  if (peek(Token::Let) || peek(Token::Fn)) {
    // #TODO: allow the default visibility to be set by the source code.
    return parseDefinition(mir, false);
  }

  auto affix = parseAffix(mir);
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  return affix;
}

Result<ir::detail::Parameter> MirParser::parseVisibility(ir::Mir &mir) {
  if (expect(Token::Public)) {
    return parseDefinition(mir, true);
  }

  if (expect(Token::Private)) {
    return parseDefinition(mir, false);
  }

  return recover(Error::Kind::ExpectedVisibility);
}

Result<ir::detail::Parameter> MirParser::parseDefinition(ir::Mir &mir,
                                                         bool visibility) {
  if (peek(Token::Let)) {
    return parseLet(mir, visibility);
  }

  if (peek(Token::Fn)) {
    return parseFunction(mir, visibility);
  }

  return recover(Error::Kind::ExpectedDefinition);
}

Result<ir::detail::Parameter> MirParser::parseLet(ir::Mir &mir,
                                                  bool visibility) {
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

  auto affix = parseAffix(mir);
  if (!affix) {
    return affix;
  }

  if (!expect(Token::Semicolon)) {
    return recover(Error::Kind::ExpectedSemicolon);
  }

  auto rhs_loc = location();
  Location let_loc{lhs_loc, rhs_loc};
  return mir.emplaceLet(source(let_loc), attributes, id, annotation,
                        affix.value());
}

Result<ir::detail::Parameter> MirParser::parseFunction(ir::Mir &mir,
                                                       bool visibility) {
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
      return Error{Error::Kind::ExpectedIdentifier, location(), text()};
    }

    auto arg_name = m_env->getIdentifier(text());
    next();

    if (!expect(Token::Colon)) {
      return Error{Error::Kind::ExpectedColon, location(), text()};
    }

    auto result = parseType();
    if (!result) {
      return result;
    }
    auto type = result.value();

    return {arg_name, Attributes{}, type};
  };

  auto parseArgumentList = [&]() -> Result<FormalArguments> {
    FormalArguments formal_arguments;

    if (!expect(Token::BeginParen)) {
      return Error{Error::Kind::ExpectedBeginParen, location(), text()};
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
      return Error{Error::Kind::ExpectedEndParen, location(), text()};
    }

    return formal_arguments;
  };

  auto arguments_result = parseArgumentList();
  if (!arguments_result) {
    return recover(std::move(arguments_result.error()));
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
  ir::Function::Body body;
  if (!expect(Token::BeginBrace)) {
    return recover(Error::Kind::ExpectedBeginBrace);
  }

  while (!peek(Token::EndBrace)) {
    ir::Mir expression;
    auto result = parseTerm(expression);
    if (!result) {
      return result;
    }
    body.emplace_back(std::move(expression));
  }

  if (!expect(Token::EndBrace)) {
    return recover(Error::Kind::ExpectedEndBrace);
  }

  auto rhs_loc = location();
  Location fn_loc{lhs_loc, rhs_loc};
  return mir.emplaceFunction(source(fn_loc), attributes, name,
                             std::move(arguments), annotation, std::move(body));
}

Result<ir::detail::Parameter> MirParser::parseAffix(ir::Mir &mir) {
  auto call = parseBasic(mir);
  if (!call) {
    return call;
  }

  if (isBinop(m_current_token)) {
    return parseBinop(mir, call.value(), {0});
  }

  return call;
}

// Result<ir::detail::Parameter> MirParser::parseCall(ir::Mir &mir) {
//  auto lhs_loc = location();
// return parseBasic(mir);
// if (!basic) {
//   return basic;
// }

// if (!expect(Token::BeginParen)) {
//   return basic;
// }

// ir::Call::Arguments arguments;

// if (!peek(Token::EndParen)) {
//   do {
//     auto result = parseAffix(mir);
//     if (!result) {
//       return result;
//     }

//     arguments.emplace_back(result.value());
//   } while (expect(Token::Comma));
// }

// if (!expect(Token::EndParen)) {
//   return recover(Error::Kind::ExpectedEndParen);
// }

// auto rhs_loc = location();
// Location call_loc{lhs_loc, rhs_loc};
// return mir.emplaceCall(source(call_loc), basic.value(),
// std::move(arguments));
// }

Result<ir::detail::Parameter> MirParser::parseBinop(ir::Mir &mir,
                                                    ir::detail::Parameter left,
                                                    BinopPrecedence p) {
  auto lhs_loc = location();
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

    auto right = parseBasic(mir);
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

    auto rhs_loc = location();
    Location binop_loc{lhs_loc, rhs_loc};
    result =
        mir.emplaceBinop(source(binop_loc), op, result.value(), right.value());
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

    // case Token::BackSlash:
    //   return parseLambda(mir);

  default:
    return recover(Error::Kind::ExpectedBasic);
  }
}

Result<ir::detail::Parameter> MirParser::parseNil(ir::Mir &mir) {
  auto sl = source();
  next();
  return mir.emplaceImmediate(sl);
}

Result<ir::detail::Parameter> MirParser::parseTrue(ir::Mir &mir) {
  auto sl = source();
  next();
  return mir.emplaceImmediate(sl, true);
}

Result<ir::detail::Parameter> MirParser::parseFalse(ir::Mir &mir) {
  auto sl = source();
  next();
  return mir.emplaceImmediate(sl, false);
}

Result<ir::detail::Parameter> MirParser::parseInteger(ir::Mir &mir) {
  auto sl = source();
  auto value = fromString<int>(text());
  next();
  return mir.emplaceImmediate(sl, value);
}

Result<ir::detail::Parameter> MirParser::parseVariable(ir::Mir &mir) {
  auto sl = source();
  auto name = m_env->getIdentifier(text());
  next();
  return mir.emplaceImmediate(sl, name);
}

Result<ir::detail::Parameter> MirParser::parseUnop(ir::Mir &mir) {
  auto lhs_loc = location();
  auto op = m_current_token;
  next();

  auto right = parseBasic(mir);
  if (!right) {
    return right;
  }

  auto rhs_loc = location();
  Location unop_loc{lhs_loc, rhs_loc};
  return mir.emplaceUnop(m_lexer.source(unop_loc), op, right.value());
}

Result<ir::detail::Parameter> MirParser::parseParens(ir::Mir &mir) {
  auto lhs_loc = location();
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

  auto rhs_loc = location();
  Location parens_loc{lhs_loc, rhs_loc};
  return mir.emplaceParens(source(parens_loc), affix.value());
}

// Result<ir::detail::Parameter> MirParser::parseLambda(ir::Mir &mir) {
//   auto lhs_loc = location();
//   if (!expect(Token::BackSlash)) {
//     return recover(Error::Kind::ExpectedBackSlash);
//   }

//   auto parseArgument = [&]() -> Result<FormalArgument> {
//     if (!peek(Token::Identifier)) {
//       return recover(Error::Kind::ExpectedIdentifier);
//     }

//     auto name = m_env->getIdentifier(text());
//     next();

//     if (!expect(Token::Colon)) {
//       return recover(Error::Kind::ExpectedColon);
//     }

//     auto annotation = parseType();
//     if (!annotation) {
//       return annotation.error();
//     }
//     return {name, Attributes{}, annotation.value()};
//   };

//   FormalArguments arguments;
//   if (peek(Token::Identifier)) {
//     do {
//       auto result = parseArgument();
//       if (!result) {
//         return result.error();
//       }

//       arguments.emplace_back(result.value());
//     } while (expect(Token::Comma));
//   }

//   std::optional<type::Ptr> annotation;
//   if (expect(Token::RightArrow)) {
//     auto result = parseType();
//     if (!result) {
//       return result.error();
//     }

//     annotation = result.value();
//   }

//   if (!expect(Token::EqualsRightArrow)) {
//     return recover(Error::Kind::ExpectedEqualsRightArrow);
//   }

//   ir::Mir body;
//   auto result = parseAffix(body);
//   if (!result) {
//     return result;
//   }

//   auto rhs_loc = location();
//   Location lambda_loc{lhs_loc, rhs_loc};
//   return mir.emplaceLambda(source(lambda_loc), std::move(arguments),
//   annotation,
//                            std::move(body));
// }

Result<type::Ptr> MirParser::parseType() {
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
Result<type::Ptr> MirParser::parseNilType() {
  next();
  return m_env->getNilType();
}

Result<type::Ptr> MirParser::parseBooleanType() {
  next();
  return m_env->getBooleanType();
}

Result<type::Ptr> MirParser::parseIntegerType() {
  next();
  return m_env->getIntegerType();
}

Result<type::Ptr> MirParser::parseFunctionType() {
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
