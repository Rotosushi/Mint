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

#include "ir/detail/IrBase.hpp"
#include "ir/value/Lambda.hpp"
#include "ir/value/Scalar.hpp"

namespace mint::ir {
class Value {
public:
  using Variant = std::variant<Scalar, Lambda>;

private:
  Variant m_variant;

public:
  Value() noexcept : m_variant(std::in_place_type<Scalar>) {}
  Value(bool boolean) noexcept
      : m_variant(std::in_place_type<Scalar>, boolean) {}
  Value(int integer) noexcept
      : m_variant(std::in_place_type<Scalar>, integer) {}
  Value(Scalar scalar) noexcept
      : m_variant(std::in_place_type<Scalar>, scalar) {}
  Value(SourceLocation *sl, FormalArguments arguments,
        std::optional<type::Ptr> annotation, Mir body) noexcept
      : m_variant(std::in_place_type<Lambda>, sl, std::move(arguments),
                  annotation, std::move(body)) {}
  Value(Value const &other) noexcept = default;
  Value(Value &&other) noexcept = default;
  auto operator=(Value const &other) noexcept -> Value & = default;
  auto operator=(Value &&other) noexcept -> Value & = default;
  ~Value() noexcept = default;

  [[nodiscard]] Variant &variant() noexcept { return m_variant; }

  template <class T> [[nodiscard]] bool holds() const noexcept {
    return std::holds_alternative<T>(m_variant);
  }

  template <class T> [[nodiscard]] T &get() noexcept {
    return std::get<T>(m_variant);
  }
};
} // namespace mint::ir
