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
class TypeInterner {
  Type boolean_type;
  Type integer_type;
  Type nil_type;

public:
  TypeInterner() noexcept
      : boolean_type{std::in_place_type<Type::Boolean>},
        integer_type{std::in_place_type<Type::Integer>},
        nil_type{std::in_place_type<Type::Nil>} {}

  auto getBooleanType() const noexcept { return &boolean_type; }
  auto getIntegerType() const noexcept { return &integer_type; }
  auto getNilType() const noexcept { return &nil_type; }
};
} // namespace mint
