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

#include "scan/Location.hpp"
#include "scan/Token.hpp"

#include "type/Type.hpp"

namespace mint {
struct Ast {
  struct Affix {
    Ast *affix;
    Affix(Ast *affix) noexcept : affix(affix) {}
  };

  struct Type {
    mint::Type::Pointer type;
    Type(mint::Type::Pointer type) noexcept : type(type) {}
  };

  struct Let {
    Identifier id;
    Ast *term;

    Let(Identifier id, Ast *term) noexcept : id(id), term(term) {}
  };

  struct Binop {
    Token op;
    Ast *left;
    Ast *right;

    Binop(Token op, Ast *left, Ast *right) noexcept
        : op(op), left(left), right(right) {}
  };

  struct Unop {
    Token op;
    Ast *right;

    Unop(Token op, Ast *right) noexcept : op(op), right(right) {}
  };

  struct Parens {
    Ast *ast;

    Parens(Ast *ast) noexcept : ast{ast} {}
  };

  struct Variable {
    Identifier name;

    Variable(Identifier name) noexcept : name{name} {}
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

  using Data =
      std::variant<Affix, Type, Let, Binop, Unop, Variable, Parens, Value>;
  Data data;
  Location location;

private:
  template <class T, class... Args>
  constexpr explicit Ast(Location location, std::in_place_type_t<T> type,
                         Args &&...args)
      : data(type, std::forward<Args>(args)...), location(location) {}

  friend class AstAllocator;
};

template <typename T> auto isa(Ast *ast) -> bool {
  return std::holds_alternative<T>(ast->data);
}

template <typename T> auto isa(Ast::Value *value) -> bool {
  return std::holds_alternative<T>(value->data);
}

/*
  it is a bit idiosyncratic to return a pointer
  when we are asserting that the get needs to succeed.
  when we could return a nullptr.
*/
template <typename T> auto get(Ast *ast) -> T * {
  MINT_ASSERT(isa<T>(ast));
  return std::get_if<T>(&ast->data);
}

template <typename T> auto get(Ast::Value *value) -> T * {
  MINT_ASSERT(isa<T>(value));
  return std::get_if<T>(&value->data);
}

} // namespace mint
