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
#include <variant>

#include "adt/Identifier.hpp"

#include "scan/Location.hpp"
#include "scan/Token.hpp"

#include "type/Type.hpp"

namespace mint {
struct Ast {
  using Pointer = std::shared_ptr<Ast>;

  struct Term {
    Location location;
    Ast::Pointer affix;
    Term(Location location, Ast::Pointer affix) noexcept
        : location(location), affix(affix) {}
  };

  struct Type {
    Location location;
    mint::Type::Pointer type;
    Type(Location location, mint::Type::Pointer type) noexcept
        : location(location), type(type) {}
  };

  struct Let {
    Location location;
    Identifier id;
    Ast::Pointer term;

    Let(Location location, Identifier id, Ast::Pointer term) noexcept
        : location(location), id(id), term(term) {}
  };

  struct Binop {
    Location location;
    Token op;
    Ast::Pointer left;
    Ast::Pointer right;

    Binop(Location location, Token op, Ast::Pointer left,
          Ast::Pointer right) noexcept
        : location(location), op(op), left(left), right(right) {}
  };

  struct Unop {
    Location location;
    Token op;
    Ast::Pointer right;

    Unop(Location location, Token op, Ast::Pointer right) noexcept
        : location(location), op(op), right(right) {}
  };

  struct Parens {
    Location location;
    Ast::Pointer ast;

    Parens(Location location, Ast::Pointer ast) noexcept
        : location(location), ast{ast} {}
  };

  struct Variable {
    Location location;
    Identifier name;

    Variable(Location location, Identifier name) noexcept
        : location(location), name{name} {}
  };

  struct Module {
    Location location;
    std::vector<Ast::Pointer> expressions;

    Module(Location location, std::vector<Ast::Pointer> expressions) noexcept
        : location(location), expressions(std::move(expressions)) {}
  };

  struct Value {
    struct Boolean {
      Location location;
      bool value;

      Boolean(Location location, bool value) noexcept
          : location(location), value{value} {}
    };

    struct Integer {
      Location location;
      int value;

      Integer(Location location, int value) noexcept
          : location(location), value{value} {}
    };

    struct Nil {
      Location location;
      bool value = false;

      Nil(Location location) noexcept : location(location) {}
    };

    using Data = std::variant<Boolean, Integer, Nil>;
    Data data;

    template <class T, class... Args>
    constexpr explicit Value(std::in_place_type_t<T> type, Args &&...args)
        : data(type, std::forward<Args>(args)...) {}
  };

  using Data =
      std::variant<Term, Type, Let, Binop, Unop, Variable, Parens, Value>;
  Data data;

private:
  mint::Type::Pointer type_cache;

public:
  template <class T, class... Args>
  constexpr explicit Ast(std::in_place_type_t<T> type, Args &&...args)
      : data(type, std::forward<Args>(args)...) {}

  std::optional<mint::Type::Pointer> cached_type() noexcept {
    if (type_cache == nullptr) {
      return std::nullopt;
    }
    return type_cache;
  }
};

template <typename T> auto isa(Ast const *ast) -> bool {
  MINT_ASSERT(ast != nullptr);
  return std::holds_alternative<T>(ast->data);
}

template <typename T> auto isa(Ast::Value const *value) -> bool {
  MINT_ASSERT(value != nullptr);
  return std::holds_alternative<T>(value->data);
}

/*
  it is a bit idiosyncratic to return a pointer
  when we are asserting that the get needs to succeed.
  when we could return a nullptr.
  however, I like this usage of pointers more, as
  we aren't creating nullptrs to unintentionally deref later.
*/
template <typename T> auto get(Ast *ast) -> T * {
  MINT_ASSERT(isa<T>(ast));
  return std::get_if<T>(&ast->data);
}

template <typename T> auto get(Ast const *ast) -> T const * {
  MINT_ASSERT(isa<T>(ast));
  return std::get_if<T>(&ast->data);
}

template <typename T> auto get(Ast::Value *value) -> T * {
  MINT_ASSERT(isa<T>(value));
  return std::get_if<T>(&value->data);
}

template <typename T> auto get(Ast::Value const *value) -> T const * {
  MINT_ASSERT(isa<T>(value));
  return std::get_if<T>(&value->data);
}

template <typename T> auto get_value(Ast *ast) -> T * {
  auto value = get<Ast::Value>(ast);
  return get<T>(value);
}

template <typename T> auto get_value(Ast const *ast) -> T const * {
  auto value = get<Ast::Value>(ast);
  return get<T>(value);
}

class AstValueLocationVisitor {
public:
  constexpr auto operator()(Ast::Value const &value) const noexcept
      -> Location {
    return std::visit(*this, value.data);
  }

  constexpr auto operator()(Ast::Value::Boolean const &boolean) const noexcept
      -> Location {
    return boolean.location;
  }

  constexpr auto operator()(Ast::Value::Integer const &integer) const noexcept
      -> Location {
    return integer.location;
  }

  constexpr auto operator()(Ast::Value::Nil const &nil) const noexcept
      -> Location {
    return nil.location;
  }
};

inline constexpr AstValueLocationVisitor ast_value_location{};

class AstLocationVisitor {
public:
  constexpr auto operator()(Ast::Pointer const &ast) const noexcept
      -> Location {
    return std::visit(*this, ast->data);
  }

  constexpr auto operator()(Ast::Term const &affix) const noexcept -> Location {
    return affix.location;
  }

  constexpr auto operator()(Ast::Type const &type) const noexcept -> Location {
    return type.location;
  }

  constexpr auto operator()(Ast::Let const &let) const noexcept -> Location {
    return let.location;
  }

  constexpr auto operator()(Ast::Binop const &binop) const noexcept
      -> Location {
    return binop.location;
  }

  constexpr auto operator()(Ast::Unop const &unop) const noexcept -> Location {
    return unop.location;
  }

  constexpr auto operator()(Ast::Parens const &parens) const noexcept
      -> Location {
    return parens.location;
  }

  constexpr auto operator()(Ast::Variable const &variable) const noexcept
      -> Location {
    return variable.location;
  }

  constexpr auto operator()(Ast::Value const &value) const noexcept
      -> Location {
    return ast_value_location(value);
  }
};

inline constexpr AstLocationVisitor ast_location{};

} // namespace mint
