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
#include <memory>  // std::unique_ptr
#include <variant> // std::variant
#include <vector>  // std::vector

#include "utility/Assert.hpp" // MINT_ASSERT

namespace mint {
/*
  \todo if we can somehow make it so the only
  valid way of constructing a Type is via
  the Type::TypeInterner, then we can rewrite Equals
  to perform pointer equality.
  this is because type Type::TypeInterner by default
  constructs a single instance of scalar types.
  and the only way of construct a type which
  was composed of other types, would be by getting
  a scalar type from the interner first, then by
  constructing a composite type with the scalar type.
  and then you can construct composite types with that
  composite type.
  so there is never a time when you have a non-unique
  address to a type, thus pointer equality suffices for
  type equality even when considering within the
  context of the interner itself.
*/
struct Type {
  using Pointer = Type const *;

  struct Boolean {};
  struct Integer {};
  struct Nil {};

  using Data = std::variant<Boolean, Integer, Nil>;
  Data data;

  template <class T, class... Args>
  constexpr explicit Type(std::in_place_type_t<T> type, Args &&...args)
      : data(type, std::forward<Args>(args)...) {}
};

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

/*

*/
class IsScalarTypeVisitor {
public:
  constexpr auto operator()(Type::Pointer type) const noexcept -> bool {
    return std::visit(*this, type->data);
  }

  constexpr auto
  operator()([[maybe_unused]] Type::Boolean const &type) const noexcept
      -> bool {
    return true;
  }

  constexpr auto
  operator()([[maybe_unused]] Type::Integer const &type) const noexcept
      -> bool {
    return true;
  }

  constexpr auto
  operator()([[maybe_unused]] Type::Nil const &type) const noexcept -> bool {
    return true;
  }
};

/*
  this is a cute trick with no argument visitors.
  they look like type_traits if you squint.

  \note we can't have the cute trick if we need an extra argument :(
    it's unsafe to default initialize a EqualsVisitor object
    because we cannot compare to a nullptr.

  \note the cute trick is less flexible than a function,
    1) if we define 'print' global struct to work on
    some type T, we cannot define print to work on
    another type U. functions have overload sets.
    structs do not. This cannot be solved with
    templates, as we need a new declaration for
    each instance of a type, (a decleration per
    kind of type, which in this case is represented
    as a runtime value.)
*/
inline constexpr IsScalarTypeVisitor isScalarType{};

class EqualsVisitor {
  Type::Pointer left;
  Type::Pointer right;

public:
  EqualsVisitor(Type::Pointer left, Type::Pointer right)
      : left{left}, right{right} {}

  auto operator()() { return std::visit(*this, right->data); }

  auto operator()([[maybe_unused]] Type::Boolean const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Boolean>(left->data);
  }

  auto operator()([[maybe_unused]] Type::Integer const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Integer>(left->data);
  }

  auto operator()([[maybe_unused]] Type::Nil const &right) const noexcept
      -> bool {
    return std::holds_alternative<Type::Nil>(left->data);
  }
};

auto equals(Type::Pointer left, Type::Pointer right) noexcept -> bool {
  EqualsVisitor visitor{left, right};
  return visitor();
}

} // namespace mint
