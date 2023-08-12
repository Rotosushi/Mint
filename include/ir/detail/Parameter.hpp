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
#include "adt/Identifier.hpp"
#include "ir/detail/Base.hpp"
#include "ir/detail/Index.hpp"
#include "ir/detail/Scalar.hpp"

namespace mint {
namespace ir {
namespace detail {
// #TODO: maybe ir/value/ isn't the best directory for this?
// the intention is that "Parameters represent an indirect value."
//
// #TODO: maybe "Parameter" isn't the best name for this?
// the intention is that "Instructions use this to refer
// to their parameters."

// represents an argument to a given MIR instruction.
// This class is meant to be trivially-copyable
// and small.
class Parameter {
public:
  using Variant = std::variant<Scalar, Identifier, Index>;

private:
  Variant m_variant;

public:
  Parameter() noexcept : m_variant() {}
  Parameter(bool boolean) noexcept
      : m_variant(std::in_place_type<Scalar>, boolean) {}
  Parameter(int integer) noexcept
      : m_variant(std::in_place_type<Scalar>, integer) {}
  Parameter(Identifier name) noexcept
      : m_variant(std::in_place_type<Identifier>, name) {}
  Parameter(Index index) noexcept
      : m_variant(std::in_place_type<Index>, index) {}
  Parameter(Parameter const &other) noexcept = default;
  Parameter(Parameter &&other) noexcept = default;
  auto operator=(Parameter const &other) noexcept -> Parameter & = default;
  auto operator=(Parameter &&other) noexcept -> Parameter & = default;
  ~Parameter() noexcept = default;

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }
};
} // namespace detail
} // namespace ir
} // namespace mint
