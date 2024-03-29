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
#include "scan/Token.hpp"

namespace mint {
auto precedence(Token token) noexcept -> BinopPrecedence {
  switch (token) {
  case Token::EqualEqual:
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

auto associativity(Token token) noexcept -> BinopAssociativity {
  switch (token) {
  case Token::EqualEqual:
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

auto isBinop(Token token) noexcept -> bool {
  switch (token) {
  case Token::EqualEqual:
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

auto isUnop(Token token) noexcept -> bool {
  switch (token) {
  case Token::Not:
  case Token::Minus:
    return true;
  default:
    return false;
  }
}

auto tokenToView(Token token) noexcept -> std::string_view {
  switch (token) {
  case Token::Error:
    return "Token::Error";
  case Token::End:
    return "Token::End";

  case Token::Let:
    return "let";
  case Token::Module:
    return "module";
  case Token::Public:
    return "public";
  case Token::Private:
    return "private";
  case Token::Import:
    return "import";
  case Token::From:
    return "from";

  case Token::Equal:
    return "=";
  case Token::Semicolon:
    return ";";
  case Token::Colon:
    return ":";
  case Token::Comma:
    return ",";
  case Token::BackSlash:
    return "\\";
  case Token::RightArrow:
    return "->";
  case Token::EqualsRightArrow:
    return "=>";
  case Token::BeginParen:
    return "(";
  case Token::EndParen:
    return ")";
  case Token::BeginBrace:
    return "{";
  case Token::EndBrace:
    return "}";

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
  case Token::EqualEqual:
    return "==";
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

  case Token::NilType:
    return "Nil";
  case Token::BooleanType:
    return "Boolean";
  case Token::IntegerType:
    return "Integer";

  case Token::Identifier:
    return "Token::Identifier";
  case Token::Integer:
    return "Token::Integer";
  case Token::Text:
    return "Token::Text";

  default:
    abort("Unknown Token");
    break;
  }
}

auto operator<<(std::ostream &out, Token token) noexcept -> std::ostream & {
  out << tokenToView(token);
  return out;
}
} // namespace mint
