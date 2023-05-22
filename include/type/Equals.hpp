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
#include "type/Type.hpp"

namespace mint {

class EqualsVisitor {
  Type::Pointer left;
  Type::Pointer right;

public:
  EqualsVisitor(Type::Pointer left, Type::Pointer right)
      : left{left}, right{right} {}

  auto operator()() { return std::visit(*this, right->data); }

  auto operator()([[maybe_unused]] Type::Boolean const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Boolean>(left->data);
  }

  auto operator()([[maybe_unused]] Type::Integer const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Integer>(left->data);
  }

  auto operator()([[maybe_unused]] Type::Nil const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Nil>(left->data);
  }
};

[[nodiscard]] inline auto equals(Type::Pointer left,
                                 Type::Pointer right) noexcept -> bool {
  EqualsVisitor visitor{left, right};
  return visitor();
}
} // namespace mint
