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
#include "ir/detail/Index.hpp"
#include "ir/value/Scalar.hpp"

namespace mint {
namespace ir {
// #TODO: maybe ir/value/ isn't the best directory for this?
// the intention is that "Parameters represent an indirect value."
//
// #TODO: maybe "Parameter" isn't the best name for this?
// the intention is that "Instructions use this to refer
// to their parameters."
class Parameter {
public:
  enum Kind {
    Scalar,
    Nil,
    Boolean,
    Integer,
    ScalarEnd,

    Variable,
    Instruction,
  };

private:
  Kind m_kind;
  // #NOTE:
  // we store Scalars and Identifiers directly,
  // to remove a level of indirection for those
  // values
  union Variant {
    ir::Scalar m_scalar;
    Identifier m_name;
    detail::Index m_index;

    Variant() noexcept : m_scalar() {}
    Variant(bool boolean) noexcept : m_scalar(boolean) {}
    Variant(int integer) noexcept : m_scalar(integer) {}
    Variant(ir::Scalar scalar) noexcept : m_scalar(scalar) {}
    Variant(Identifier name) noexcept : m_name(name) {}
    Variant(detail::Index index) noexcept : m_index(index) {}
    ~Variant() noexcept = default;
  };
  Variant m_variant;

  static assign(Kind kind, Variant &left, Variant &right) noexcept {
    switch (kind) {
    case Scalar:
    case Nil:
    case Boolean:
    case Integer:
    case ScalarEnd:
      left.m_scalar = right.m_scalar;
      break;
    case Variable:
      left.m_name = right.m_name;
      break;
    case Instruction:
      left.m_index = right.m_index;
      break;
    default:
      abort("bad Parameter Variant Kind");
    }
  }

public:
  Parameter() noexcept : m_kind(Nil), m_variant() {}
  Parameter(bool boolean) noexcept : m_kind(Boolean), m_variant(boolean) {}
  Parameter(int integer) noexcept : m_kind(Integer), m_variant(integer) {}
  Parameter(ir::Scalar scalar) noexcept : m_kind(Scalar), m_variant(scalar) {
    switch (scalar.kind()) {
    case ir::Scalar::Nil:
      m_kind = Nil;
      break;
    case ir::Scalar::Boolean:
      m_kind = Boolean;
      break;
    case ir::Scalar::Integer:
      m_kind = Integer;
      break;
    default:
      m_kind = Scalar;
      break;
    }
  }
  Parameter(Identifier name) noexcept : m_kind(Variable), m_variant(name) {}
  Parameter(detail::Index index) noexcept
      : m_kind(Instruction), m_variant(index) {}
  Parameter(Parameter const &other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  Parameter(Parameter &&other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  auto operator=(Parameter const &other) noexcept -> Parameter & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
  }
  auto operator=(Parameter &&other) noexcept -> Parameter & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
  }
  ~Parameter() noexcept = default;

  [[nodiscard]] auto kind() const noexcept -> Kind { return m_kind; }

  [[nodiscard]] auto isScalar(Kind kind = kind()) const noexcept -> bool {
    return (kind >= Scalar) && (kind <= ScalarEnd);
  }

  [[nodiscard]] auto scalar() const noexcept -> ir::Scalar {
    MINT_ASSERT(isScalar());
    return m_variant.m_scalar;
  }

  [[nodiscard]] auto scalarKind() const noexcept -> ir::Scalar::Kind {
    MINT_ASSERT(isScalar());
    return m_variant.m_scalar.kind();
  }

  [[nodiscard]] auto nil() const noexcept -> bool {
    MINT_ASSERT(kind() == Nil);
    return m_variant.m_scalar.nil();
  }

  [[nodiscard]] auto boolean() const noexcept -> bool {
    MINT_ASSERT(kind() == Boolean);
    return m_variant.m_scalar.boolean();
  }

  [[nodiscard]] auto integer() const noexcept -> int {
    MINT_ASSERT(kind() == Integer);
    return m_variant.m_scalar.integer();
  }

  [[nodiscard]] auto variable() const noexcept -> Identifier {
    MINT_ASSERT(kind() == Variable);
    return m_variant.m_name;
  }

  [[nodiscard]] auto instruction() const noexcept -> ir::Instruction & {
    MINT_ASSERT(!isScalar());
    // #NOTE: we don't check for nullptr here, because the only
    // way of validly constructing a Parameter is by passing in a reference
    // to an instruction. and we assume that references are never nullptr.
    return *(m_variant.m_instruction);
  }
};
} // namespace ir
} // namespace mint
