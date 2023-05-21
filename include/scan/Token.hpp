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
#include <string_view>

#include "utility/FatalError.hpp"

namespace mint {
enum struct Token : int {
  // utility
  Error,
  End,

  // keywords
  Let,

  // symbols
  Equal,
  Semicolon,
  LParen,
  RParen,

  // operators
  Plus,
  Minus,
  Star,
  Divide,
  Modulo,
  Not,
  And,
  Or,
  LessThan,
  LessThanOrEqual,
  QuestionEqual,
  NotEqual,
  GreaterThan,
  GreaterThanOrEqual,

  // literals
  Nil,
  True,
  False,

  // Types
  BooleanType,
  IntegerType,
  NilType,

  // regular-expressions
  Identifier,
  Integer,
};

using BinopPrecedence = int8_t;

/*

  != ?=     -> 1
  < <= >= > -> 2
  & |       -> 3
  + -       -> 4
  * / %     -> 5

*/
inline auto precedence(Token token) noexcept -> BinopPrecedence {
  switch (token) {
  case Token::QuestionEqual:
  case Token::NotEqual:
    return 1;

  case Token::LessThan:
  case Token::LessThanOrEqual:
  case Token::GreaterThanOrEqual:
  case Token::GreaterThan:
    return 2;

  case Token::And:
  case Token::Or:
    return 3;

  case Token::Plus:
  case Token::Minus:
    return 4;

  case Token::Star:
  case Token::Divide:
  case Token::Modulo:
    return 5;

  default:
    return -1;
  }
}

enum struct BinopAssociativity {
  None,
  Left,
  Right,
};

inline auto associativity(Token token) noexcept -> BinopAssociativity {
  switch (token) {
  case Token::QuestionEqual:
  case Token::NotEqual:
  case Token::LessThan:
  case Token::LessThanOrEqual:
  case Token::GreaterThanOrEqual:
  case Token::GreaterThan:
  case Token::And:
  case Token::Or:
  case Token::Plus:
  case Token::Minus:
  case Token::Star:
  case Token::Divide:
  case Token::Modulo:
    return BinopAssociativity::Left;
  default:
    return BinopAssociativity::None;
  }
}

inline auto isBinop(Token token) noexcept -> bool {
  switch (token) {
  case Token::QuestionEqual:
  case Token::NotEqual:
  case Token::LessThan:
  case Token::LessThanOrEqual:
  case Token::GreaterThanOrEqual:
  case Token::GreaterThan:
  case Token::And:
  case Token::Or:
  case Token::Plus:
  case Token::Minus:
  case Token::Star:
  case Token::Divide:
  case Token::Modulo:
    return true;
  default:
    return false;
  }
}

inline auto isUnop(Token token) noexcept -> bool {
  switch (token) {
  case Token::Not:
  case Token::Minus:
    return true;
  default:
    return false;
  }
}

inline auto toString(Token token) noexcept -> std::string_view {
  switch (token) {
  case Token::Error:
    fatalError("Token::Error");
  case Token::End:
    fatalError("Token::End");

  case Token::Let:
    return "let";
  case Token::Equal:
    return "=";
  case Token::Semicolon:
    return ";";
  case Token::LParen:
    return "(";
  case Token::RParen:
    return ")";
  case Token::Plus:
    return "+";
  case Token::Minus:
    return "-";
  case Token::Star:
    return "*";
  case Token::Divide:
    return "/";
  case Token::Modulo:
    return "%";
  case Token::Not:
    return "!";
  case Token::And:
    return "&";
  case Token::Or:
    return "|";
  case Token::LessThan:
    return "<";
  case Token::LessThanOrEqual:
    return "<=";
  case Token::QuestionEqual:
    return "?=";
  case Token::NotEqual:
    return "!=";
  case Token::GreaterThan:
    return "<";
  case Token::GreaterThanOrEqual:
    return "<=";
  case Token::Nil:
    return "nil";
  case Token::True:
    return "true";
  case Token::False:
    return "false";

  default:
    fatalError("Unknown Token");
    break;
  }
}

} // namespace mint
