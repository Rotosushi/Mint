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
#include "ir/detail/Immediate.hpp"
#include "ir/detail/Index.hpp"
#include "type/Type.hpp"

namespace mint {
namespace ir {
namespace detail {
// #TODO: maybe "Parameter" isn't the best name for this?
// the intention is that "Instructions use this to refer
// to their parameters. such that they receive the benefiet
// of holding onto scalar parameters directly without having
// to worry about the mechanics of that."

// represents an argument to a given MIR instruction.
// This class is meant to be trivially-copyable
// and as small as possible.
class Parameter {
public:
  using Variant = std::variant<Immediate, Index>;

private:
  type::Ptr m_cached_type;
  Variant m_variant;

public:
  Parameter() noexcept
      : m_cached_type(nullptr), m_variant(std::in_place_type<Immediate>) {}
  Parameter(bool boolean) noexcept
      : m_cached_type(nullptr),
        m_variant(std::in_place_type<Immediate>, boolean) {}
  Parameter(int integer) noexcept
      : m_cached_type(nullptr),
        m_variant(std::in_place_type<Immediate>, integer) {}
  Parameter(Identifier name) noexcept
      : m_cached_type(nullptr), m_variant(std::in_place_type<Immediate>, name) {
  }
  Parameter(Immediate immediate) noexcept
      : m_cached_type(nullptr),
        m_variant(std::in_place_type<Immediate>, immediate) {}
  Parameter(Index index) noexcept
      : m_cached_type(nullptr), m_variant(std::in_place_type<Index>, index) {}
  Parameter(Parameter const &other) noexcept = default;
  Parameter(Parameter &&other) noexcept = default;
  auto operator=(Parameter const &other) noexcept -> Parameter & = default;
  auto operator=(Parameter &&other) noexcept -> Parameter & = default;
  ~Parameter() noexcept = default;

  [[nodiscard]] type::Ptr cachedType() const noexcept { return m_cached_type; }
  type::Ptr cachedType(type::Ptr type) noexcept { return m_cached_type = type; }

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }
};
} // namespace detail
} // namespace ir
} // namespace mint
