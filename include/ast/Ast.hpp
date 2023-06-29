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

#include "adt/Attributes.hpp"
#include "error/Result.hpp"
#include "scan/Location.hpp"
#include "type/Type.hpp"
#include "utility/Allocator.hpp"
#include "utility/Assert.hpp"

namespace mint {
class Environment;

namespace ast {
class Ast;
using Ptr = std::shared_ptr<Ast>;

class Ast : public std::enable_shared_from_this<Ast> {
public:
  enum class Kind {
    // Definitions
    Definition,
    Let,
    // Function,
    EndDefinition,

    // Values
    Value,
    Nil,
    Boolean,
    Integer,
    EndValue,

    // Syntax
    Term,
    Parens,

    // Semantics
    Module,
    Import,
    Binop,
    Unop,
    Variable,
  };

private:
  mutable type::Ptr m_cached_type;
  Kind m_kind;
  Attributes m_attributes;
  Location m_location;

protected:
  Ast(Kind kind, Attributes attributes, Location location) noexcept
      : m_cached_type{nullptr}, m_kind{kind}, m_attributes{attributes},
        m_location{location} {}

public:
  virtual ~Ast() noexcept = default;

  void setCachedType(type::Ptr type) const noexcept { m_cached_type = type; }

  [[nodiscard]] auto cachedType() const noexcept { return m_cached_type; }
  [[nodiscard]] auto cachedTypeOrAssert() const noexcept {
    MINT_ASSERT(m_cached_type != nullptr);
    return m_cached_type;
  }
  [[nodiscard]] auto kind() const noexcept { return m_kind; }
  [[nodiscard]] auto attributes() const noexcept { return m_attributes; }
  [[nodiscard]] auto location() const noexcept { return m_location; }

  virtual Ptr clone(Allocator &allocator) const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;

  virtual Result<type::Ptr> typecheck(Environment &env) const noexcept = 0;
  virtual Result<ast::Ptr> evaluate(Environment &env) noexcept = 0;
};

inline auto operator<<(std::ostream &out, ast::Ptr const &ast) noexcept
    -> std::ostream & {
  ast->print(out);
  return out;
}
} // namespace ast
/*
struct Ast {
  using Ptr = std::shared_ptr<Ast>;

  struct Let {
    Attributes attributes;
    Location location;
    Identifier id;
    std::optional<Type::Ptr> annotation;
    Ast::Ptr term;

    Let(Attributes attributes, Location location, Identifier id,
        std::optional<Type::Ptr> annotation, Ast::Ptr term) noexcept
        : attributes(attributes), location(location), id(id),
          annotation(annotation), term(term) {}
  };

  struct Binop {
    Attributes attributes;
    Location location;
    Token op;
    Ast::Ptr left;
    Ast::Ptr right;

    Binop(Attributes attributes, Location location, Token op, Ast::Ptr left,
          Ast::Ptr right) noexcept
        : attributes(attributes), location(location), op(op), left(left),
          right(right) {}
  };

  struct Unop {
    Attributes attributes;
    Location location;
    Token op;
    Ast::Ptr right;

    Unop(Attributes attributes, Location location, Token op,
         Ast::Ptr right) noexcept
        : attributes(attributes), location(location), op(op), right(right) {}
  };

  struct Term {
    Attributes attributes;
    Location location;
    std::optional<Ast::Ptr> ast;

    Term(Attributes attributes, Location location,
         std::optional<Ast::Ptr> ast) noexcept
        : attributes(attributes), location(location), ast{ast} {}
  };

  struct Parens {
    Attributes attributes;
    Location location;
    Ast::Ptr ast;

    Parens(Attributes attributes, Location location, Ast::Ptr ast) noexcept
        : attributes(attributes), location(location), ast{ast} {}
  };

  struct Variable {
    Attributes attributes;
    Location location;
    Identifier name;

    Variable(Attributes attributes, Location location, Identifier name) noexcept
        : attributes(attributes), location(location), name{name} {}
  };

  struct Module {
    Attributes attributes;
    Location location;
    Identifier name;
    std::vector<Ast::Ptr> expressions;

    Module(Attributes attributes, Location location, Identifier name,
           std::vector<Ast::Ptr> expressions) noexcept
        : attributes(attributes), location(location), name(name),
          expressions(std::move(expressions)) {}
  };

  struct Import {
    Attributes attributes;
    Location location;
    std::string file;
    // #NOTE: #FUTURE: the 'from' mechanism seems like a
    // great candidate to implement pattern matching
    // in order to make import more expressive.
    // std::optional<Identifier> second;

    Import(Attributes attributes, Location location, std::string_view file
           ) noexcept
        : attributes(attributes), location(location), file(file)
     {}
  };

  struct Value {
    struct Boolean {
      Attributes attributes;
      Location location;
      bool value;

      Boolean(Attributes attributes, Location location, bool value) noexcept
          : attributes(attributes), location(location), value{value} {}
    };

    struct Integer {
      Attributes attributes;
      Location location;
      int value;

      Integer(Attributes attributes, Location location, int value) noexcept
          : attributes(attributes), location(location), value{value} {}
    };

    struct Nil {
      Attributes attributes;
      Location location;
      bool value = false;

      Nil(Attributes attributes, Location location) noexcept
          : attributes(attributes), location(location) {}
    };

    using Data = std::variant<Boolean, Integer, Nil>;
    Data data;

    template <class T, class... Args>
    constexpr explicit Value(std::in_place_type_t<T> type, Args &&...args)
        : data(type, std::forward<Args>(args)...) {}
  };

  using Data = std::variant<Let, Module, Import, Binop, Unop, Term, Parens,
                            Variable, Value>;
  Data data;

private:
  //  mutable mint::Type::Pointer type_cache;

public:
  template <class T, class... Args>
  constexpr explicit Ast(std::in_place_type_t<T> type, Args &&...args) noexcept
      : data(type, std::forward<Args>(args)...) {}

  template <class T, class Alloc, class... Args>
  static auto create(Alloc const &allocator, Args &&...args) noexcept {
    return std::allocate_shared<Ast, Alloc>(allocator, std::in_place_type<T>,
                                            std::forward<Args>(args)...);
  }


    void setCachedType(mint::Type::Pointer type) const noexcept {
      MINT_ASSERT(type != nullptr);
      type_cache = type;
    }
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


  it is a bit idiosyncratic to return a pointer
  when we are asserting that the get needs to succeed.
  when we could return a nullptr.
  however, I like this usage of pointers more, as
  we aren't creating nullptrs to unintentionally deref later.

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
  auto operator()(Ast::Value const &value) const noexcept -> Location {
    return std::visit(*this, value.data);
  }

  auto operator()(Ast::Value::Boolean const &boolean) const noexcept
      -> Location {
    return boolean.location;
  }

  auto operator()(Ast::Value::Integer const &integer) const noexcept
      -> Location {
    return integer.location;
  }

  auto operator()(Ast::Value::Nil const &nil) const noexcept -> Location {
    return nil.location;
  }
};

inline AstValueLocationVisitor ast_value_location{};

class AstLocationVisitor {
public:
  auto operator()(Ast::Ptr const &ast) const noexcept -> Location {
    return std::visit(*this, ast->data);
  }

  auto operator()(Ast const &ast) const noexcept -> Location {
    return std::visit(*this, ast.data);
  }

  auto operator()(Ast::Let const &let) const noexcept -> Location {
    return let.location;
  }

  auto operator()(Ast::Module const &m) const noexcept -> Location {
    return m.location;
  }

  auto operator()(Ast::Import const &i) const noexcept -> Location {
    return i.location;
  }

  auto operator()(Ast::Binop const &binop) const noexcept -> Location {
    return binop.location;
  }

  auto operator()(Ast::Unop const &unop) const noexcept -> Location {
    return unop.location;
  }

  auto operator()(Ast::Term const &term) const noexcept -> Location {
    return term.location;
  }

  auto operator()(Ast::Parens const &parens) const noexcept -> Location {
    return parens.location;
  }

  auto operator()(Ast::Variable const &variable) const noexcept -> Location {
    return variable.location;
  }

  auto operator()(Ast::Value const &value) const noexcept -> Location {
    return ast_value_location(value);
  }
};

inline AstLocationVisitor ast_location{};
*/

} // namespace mint
