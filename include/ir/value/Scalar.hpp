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

namespace mint {
namespace ir {
class Scalar {
public:
  using Variant = std::variant<std::monostate, bool, int>;

private:
  Variant m_variant;

public:
  Scalar() noexcept = default;
  Scalar(bool boolean) noexcept
      : m_variant(std::in_place_type<bool>, boolean) {}
  Scalar(int integer) noexcept : m_variant(std::in_place_type<int>, integer) {}
  Scalar(Scalar const &other) noexcept = default;
  Scalar(Scalar &&other) noexcept = default;
  auto operator=(Scalar const &other) noexcept -> Scalar & = default;
  auto operator=(Scalar &&other) noexcept -> Scalar & = default;
  ~Scalar() noexcept = default;

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }

  template <class T> [[nodiscard]] bool holds() const noexcept {
    return std::holds_alternative<T>(m_variant);
  }

  template <class T> [[nodiscard]] T &get() noexcept {
    return std::get<T>(m_variant);
  }
};
} // namespace ir
} // namespace mint
