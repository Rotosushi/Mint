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

#include <ostream>
#include <string_view>

#include "utility/Abort.hpp"

namespace mint {
enum struct Token : int {
  // utility
  Error,
  End,

  // keywords
  Let,
  Module,
  Public,
  Private,
  Import,
  From,

  // symbols
  Equal,
  Semicolon,
  Colon,
  BeginParen,
  EndParen,
  BeginBrace,
  EndBrace,

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
  EqualEqual,
  NotEqual,
  GreaterThan,
  GreaterThanOrEqual,

  // literal values
  Nil,
  True,
  False,

  // types
  BooleanType,
  IntegerType,
  NilType,

  // regular-expressions
  Identifier,
  Integer,
  Text,
};

using BinopPrecedence = unsigned char;
enum struct BinopAssociativity {
  None,
  Left,
  Right,
};

/*

  != ==     -> 1
  < <= >= > -> 2
  & |       -> 3
  + -       -> 4
  * / %     -> 5

*/
auto precedence(Token token) noexcept -> BinopPrecedence;
auto associativity(Token token) noexcept -> BinopAssociativity;
auto isBinop(Token token) noexcept -> bool;
auto isUnop(Token token) noexcept -> bool;
auto tokenToView(Token token) noexcept -> std::string_view;

auto operator<<(std::ostream &out, Token token) noexcept -> std::ostream &;

} // namespace mint
