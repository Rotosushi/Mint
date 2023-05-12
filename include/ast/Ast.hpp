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
#include <optional>
#include <string_view>
#include <variant>

#include "adt/StringSet.hpp"

#include "type/Type.hpp"

namespace mint {
class Ast {
public:
  using Pointer = Ast *;

  class Bind {
  private:
    InternedString id;
    std::optional<Type::Pointer> type_annotation;
    Pointer term;

  public:
    Bind(InternedString id, std::optional<Type::Pointer> type_anno,
         Pointer term) noexcept
        : id{id}, type_annotation{type_anno}, term{term} {}

    [[nodiscard]] auto Name() noexcept -> InternedString & { return id; }
    [[nodiscard]] auto Name() const noexcept -> InternedString const & {
      return id;
    }
    [[nodiscard]] auto Anno() noexcept -> std::optional<Type::Pointer> & {
      return type_annotation;
    }
    [[nodiscard]] auto Anno() const noexcept
        -> std::optional<Type::Pointer> const & {
      return type_annotation;
    }
    [[nodiscard]] auto Term() noexcept -> Pointer { return term; }
    [[nodiscard]] auto Term() const noexcept -> Pointer { return term; }
  };

  class Binop {};

  class Unop {};

  class Value {
  public:
    class Boolean {
      bool value;

    public:
      Boolean(bool value) noexcept : value{value} {}

      operator bool() const noexcept { return value; }
      auto get() const noexcept { return value; }
    };

    class Integer {
      int value;

    public:
      Integer(int value) noexcept : value{value} {}

      operator int() const noexcept { return value; }
      auto get() const noexcept { return value; }
    };

    class Nil {
    public:
      operator bool() const noexcept { return false; }
      auto get() const noexcept { return false; }
    };

    using Data = std::variant<Boolean, Integer, Nil>;

  private:
    Data data;

  public:
    template <class T, class... Args>
    constexpr explicit Value(std::in_place_type_t<T> type, Args &&...args)
        : data(type, std::forward<Args>(args)...) {}

    [[nodiscard]] auto variant() noexcept -> Data & { return data; }
    [[nodiscard]] auto variant() const noexcept -> Data const & { return data; }
  };

  class Variable {
  private:
    InternedString id;

  public:
    Variable(InternedString name) noexcept : id{name} {}

    [[nodiscard]] auto name() noexcept -> InternedString { return id; }
  };
};
} // namespace mint
