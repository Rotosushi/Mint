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
#include <cstdint>

namespace mint {
namespace ir {
namespace detail {
// #NOTE: since we are flattening an Ast into
// an array, we need some way of pushing elements
// onto an array without destroying the references
// element's have to eachother in the process.
// since actual references are not preserved,
// and we know we are within an array, we can store
// the offset into said array as the 'reference'
// to said element. The type itself is a struct
// for type-safety, and convience.

// an index into an array of Instructions.
// #NOTE: this index is only valid for the array
// in which the instruction resides.
class Index {
  std::size_t m_index;

public:
  Index() noexcept : m_index(0) {}
  Index(std::size_t index) noexcept : m_index(index) {}
  Index(Index const &other) noexcept = default;
  Index(Index &&other) noexcept = default;
  auto operator=(Index const &other) noexcept -> Index & = default;
  auto operator=(Index &&other) noexcept -> Index & = default;
  ~Index() noexcept = default;

  [[nodiscard]] auto operator+(Index const &other) const noexcept -> Index {
    return m_index + other.m_index;
  }
  [[nodiscard]] auto operator-(Index const &other) const noexcept -> Index {
    return m_index - other.m_index;
  }
  [[nodiscard]] auto operator+=(Index const &other) noexcept -> Index {
    return m_index += other.m_index;
  }
  [[nodiscard]] auto operator-=(Index const &other) noexcept -> Index {
    return m_index -= other.m_index;
  }

  [[nodiscard]] auto operator<=>(Index const &other) const noexcept = default;

  [[nodiscard]] auto index() const noexcept -> std::size_t { return m_index; }
};
} // namespace detail
} // namespace ir
} // namespace mint
