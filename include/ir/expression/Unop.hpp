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

namespace mint {
namespace ir {
class Unop {
public:
  enum Op {
    Neg,
    Not,
  };

private:
  Op m_op;
  detail::Parameter m_right;

public:
  Unop(Op op, detail::Parameter right) noexcept : m_op(op), m_right(right) {}
  Unop(Unop const &other) noexcept = default;
  Unop(Unop &&other) noexcept = default;
  auto operator=(Unop const &other) noexcept -> Unop & = default;
  auto operator=(Unop &&other) noexcept -> Unop & = default;
  ~Unop() noexcept = default;

  [[nodiscard]] auto op() const noexcept -> Op { return m_op; }
  [[nodiscard]] auto right() const noexcept -> detail::Parameter {
    return m_right;
  }
};
} // namespace ir

} // namespace mint
