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
#include <memory>
#include <optional>
#include <string_view>
#include <variant>

#include "adt/StringSet.hpp"

#include "type/Type.hpp"

#include "scan/Location.hpp"
#include "scan/Token.hpp"

namespace mint {
struct Ast {
  using Pointer = std::unique_ptr<Ast>;

  struct Type {
    mint::Type type;
    Type(mint::Type type) noexcept : type(type) {}
  };

  struct Let {
    Identifier id;
    std::optional<Type> type_annotation;
    Pointer term;

    Let(Identifier id, std::optional<Type> type_anno, Pointer term) noexcept
        : id{id}, type_annotation{type_anno}, term{std::move(term)} {}
  };

  struct Binop {
    Token op;
    Pointer left;
    Pointer right;

    Binop(Token op, Pointer left, Pointer right) noexcept
        : op(op), left(std::move(left)), right(std::move(right)) {}
  };

  struct Unop {
    Token op;
    Pointer right;

    Unop(Token op, Pointer right) noexcept : op(op), right(std::move(right)) {}
  };

  struct Value {
    struct Boolean {
      bool value;

      Boolean(bool value) noexcept : value{value} {}
    };

    struct Integer {
      int value;

      Integer(int value) noexcept : value{value} {}
    };

    struct Nil {
      bool value = false;
    };

    using Data = std::variant<Boolean, Integer, Nil>;
    Data data;

    template <class T, class... Args>
    constexpr explicit Value(std::in_place_type_t<T> type, Args &&...args)
        : data(type, std::forward<Args>(args)...) {}
  };

  struct Variable {
    Identifier name;

    Variable(Identifier name) noexcept : name{name} {}
  };

  using Data = std::variant<Type, Let, Binop, Unop, Value, Variable>;
  Data data;
  Location location;

  template <class T, class... Args>
  constexpr explicit Ast(Location location, std::in_place_type_t<T> type,
                         Args &&...args)
      : data(type, std::forward<Args>(args)...), location(location) {}

  [[nodiscard]] static auto createType(Location location, Type type) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Ast::Type>, type);
  }

  [[nodiscard]] static auto createLet(Location location, Identifier name,
                                      std::optional<Type> annotation,
                                      Ast::Pointer term) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Let>, name,
                                 annotation, std::move(term));
  }

  [[nodiscard]] static auto createBinop(Location location, Token op,
                                        Pointer left, Pointer right) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Binop>, op,
                                 std::move(left), std::move(right));
  }

  [[nodiscard]] static auto createUnop(Location location, Token op,
                                       Pointer right) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Unop>, op,
                                 std::move(right));
  }

  [[nodiscard]] static auto createBoolean(Location location,
                                          bool value) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Value>,
                                 std::in_place_type<Value::Boolean>, value);
  }

  [[nodiscard]] static auto createInteger(Location location,
                                          int value) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Value>,
                                 std::in_place_type<Value::Integer>, value);
  }

  [[nodiscard]] static auto createNil(Location location) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Value>,
                                 std::in_place_type<Value::Nil>);
  }

  [[nodiscard]] static auto createVariable(Location location,
                                           Identifier name) noexcept {
    return std::make_unique<Ast>(location, std::in_place_type<Variable>, name);
  }
};

} // namespace mint
