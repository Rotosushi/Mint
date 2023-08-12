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
#include "ir/detail/Parameter.hpp"
#include "scan/Token.hpp"

namespace mint {
namespace ir {
class Binop {
public:
  // enum Op {
  //   Plus,
  //   Minus,
  //   Star,
  //   Divide,
  //   Modulo,
  //   Not,
  //   And,
  //   Or,
  //   LessThan,
  //   LessThanOrEqual,
  //   EqualEqual,
  //   NotEqual,
  //   GreaterThan,
  //   GreaterThanOrEqual,
  // };

private:
  Token m_op;
  detail::Parameter m_left;
  detail::Parameter m_right;

public:
  Binop(Token op) noexcept : m_op(op) {}
  Binop(Binop const &other) noexcept = default;
  Binop(Binop &&other) noexcept = default;
  auto operator=(Binop const &other) noexcept -> Binop & = default;
  auto operator=(Binop &&other) noexcept -> Binop & = default;
  ~Binop() noexcept = default;

  [[nodiscard]] auto op() const noexcept -> Token { return m_op; }
  [[nodiscard]] auto left() noexcept -> detail::Parameter & { return m_left; }
  [[nodiscard]] auto right() noexcept -> detail::Parameter & { return m_right; }
};
} // namespace ir
} // namespace mint
