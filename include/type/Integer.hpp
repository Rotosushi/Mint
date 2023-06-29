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
namespace type {
class Integer : public Type {
public:
  Integer() noexcept : Type{Type::Kind::Integer} {}

  static auto classof(Ptr type) noexcept -> bool {
    return Type::Kind::Integer == type->kind();
  }

  [[nodiscard]] bool equals(Ptr right) const noexcept override {
    return llvm::dyn_cast<const Integer>(right) != nullptr;
  }

  void print(std::ostream &out) const noexcept override { out << "Integer"; }
};
} // namespace type
} // namespace mint
