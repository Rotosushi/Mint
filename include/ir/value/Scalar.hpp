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
#include "utility/Abort.hpp"
#include "utility/Assert.hpp"

namespace mint {
namespace ir {
class Scalar {
public:
  enum Kind {
    Boolean,
    Integer,
    Nil,
  };

private:
  Kind m_kind;

  union Variant {
    bool m_nil;
    bool m_boolean;
    int m_integer;

    Variant() noexcept : m_nil(false) {}
    Variant(bool boolean) noexcept : m_boolean(boolean) {}
    Variant(int integer) noexcept : m_integer(integer) {}
    ~Variant() noexcept = default;
  };

  Variant m_variant;

  static auto assign(Kind kind, Variant &left, Variant &right) noexcept {
    switch (kind) {
    case Boolean:
      left.m_boolean = right.m_boolean;
      break;
    case Integer:
      m_variant.m_integer = right.m_integer;
      break;
    case Nil:
      m_variant.m_nil = right.m_nil;
      break;
    default:
      abort("bad Scalar Kind");
    }
  }

public:
  Scalar() noexcept : m_kind(Nil), m_variant() {}
  Scalar(bool boolean) noexcept : m_kind(Boolean), m_variant(boolean) {}
  Scalar(int integer) noexcept : m_kind(Integer), m_variant(integer) {}
  // #NOTE: scalars are the one exception to the "No copying or moving
  // ir instructions" as a scalar never references another instruction.
  Scalar(Scalar const &other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  Scalar(Scalar &&other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  auto operator=(Scalar const &other) noexcept -> Scalar & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
    return *this;
  }
  auto operator=(Scalar &&other) noexcept -> Scalar & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
    return *this;
  }

  ~Scalar() noexcept = default;

  [[nodiscard]] auto kind() const noexcept -> Kind { return m_kind; }

  [[nodiscard]] auto nil() const noexcept -> bool {
    MINT_ASSERT(kind() == Nil);
    return m_variant.m_nil;
  }

  [[nodiscard]] auto boolean() const noexcept -> bool {
    MINT_ASSERT(kind() == Boolean);
    return m_variant.m_boolean;
  }

  [[nodiscard]] auto integer() const noexcept -> int {
    MINT_ASSERT(kind() == Integer);
    return m_variant.m_integer;
  }
};
} // namespace ir
} // namespace mint
