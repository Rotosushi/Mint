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
#include "ir/detail/Index.hpp"
#include "type/Type.hpp"

namespace mint {
namespace ir {
namespace detail {
// represents an argument to a given MIR instruction.
// This class is meant to be trivially-copyable
// and as small as possible.
class Parameter {
private:
  type::Ptr m_cached_type;
  detail::Index m_index;

public:
  Parameter(Index index) noexcept : m_cached_type(nullptr), m_index(index) {}
  Parameter(Parameter const &other) noexcept = default;
  Parameter(Parameter &&other) noexcept = default;
  auto operator=(Parameter const &other) noexcept -> Parameter & = default;
  auto operator=(Parameter &&other) noexcept -> Parameter & = default;
  ~Parameter() noexcept = default;

  [[nodiscard]] type::Ptr cachedType() const noexcept { return m_cached_type; }
  type::Ptr cachedType(type::Ptr type) noexcept { return m_cached_type = type; }

  detail::Index index() const noexcept { return m_index; }
};
} // namespace detail
} // namespace ir
} // namespace mint
