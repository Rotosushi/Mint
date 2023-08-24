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
#include <variant>

#include "adt/Identifier.hpp"
#include "ir/value/Scalar.hpp"

namespace mint {
namespace ir {
namespace detail {
class Immediate {
public:
  using Variant = std::variant<Scalar, Identifier>;

private:
  Variant m_variant;

public:
  Immediate() noexcept = default;
  Immediate(bool boolean) noexcept
      : m_variant(std::in_place_type<Scalar>, boolean) {}
  Immediate(int integer) noexcept
      : m_variant(std::in_place_type<Scalar>, integer) {}
  Immediate(Identifier name) noexcept
      : m_variant(std::in_place_type<Identifier>, name) {}
  Immediate(Immediate const &other) noexcept = default;
  Immediate(Immediate &&other) noexcept = default;
  auto operator=(Immediate const &other) noexcept -> Immediate & = default;
  auto operator=(Immediate &&other) noexcept -> Immediate & = default;
  ~Immediate() noexcept = default;

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }

  template <class T> [[nodiscard]] bool holds() const noexcept {
    return std::holds_alternative<T>(m_variant);
  }

  template <class T> [[nodiscard]] T &get() noexcept {
    return std::get<T>(m_variant);
  }
};
} // namespace detail
} // namespace ir
} // namespace mint
