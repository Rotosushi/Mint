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
#include "adt/Identifier.hpp"
#include "error/Result.hpp"
#include "scan/Location.hpp"
#include "type/Type.hpp"
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
    Syntax,
    Affix,
    Parens,
    EndSyntax,

    // Expression
    Expression,
    Binop,
    Unop,
    Variable,
    EndExpression,

    // Statement
    Statement,
    Module,
    Import,
    EndStatement,
  };

private:
  mutable Ast *m_prev_ast;
  mutable type::Ptr m_cached_type;
  Kind m_kind;
  Attributes m_attributes;
  Location m_location;

protected:
  Ast(Kind kind, Attributes attributes, Location location) noexcept
      : m_prev_ast{nullptr}, m_cached_type{nullptr}, m_kind{kind},
        m_attributes{attributes}, m_location{location} {}

  bool havePrevAst() const noexcept { return m_prev_ast != nullptr; }

  Ast *getPrevAst() const noexcept {
    MINT_ASSERT(havePrevAst());
    return m_prev_ast;
  }

public:
  virtual ~Ast() noexcept = default;

  void setPrevAst(Ast *prev_ast) const noexcept {
    MINT_ASSERT(prev_ast != nullptr);
    m_prev_ast = prev_ast;
  }
  void setCachedType(type::Ptr type) const noexcept {
    MINT_ASSERT(type != nullptr);
    m_cached_type = type;
  }

  [[nodiscard]] auto cachedType() const noexcept { return m_cached_type; }
  [[nodiscard]] auto cachedTypeOrAssert() const noexcept {
    MINT_ASSERT(m_cached_type != nullptr);
    return m_cached_type;
  }
  [[nodiscard]] auto kind() const noexcept { return m_kind; }
  [[nodiscard]] auto attributes() const noexcept { return m_attributes; }
  [[nodiscard]] auto location() const noexcept { return m_location; }

  [[nodiscard]] virtual Ptr clone() const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;

  // #NOTE: walk up the Ast, iff we find an definition,
  // then return the definitions name.
  // if we reach the top the return std::nullopt.
  // #NOTE: use-before-def relies in part on there being
  // no way of constructing an Ast which holds a
  // definition within another definition, by the
  // grammar.
  // #TODO: this isn't a name I am totally happy with.
  [[nodiscard]] virtual std::optional<Identifier>
  getDefinitionName() const noexcept = 0;

  [[nodiscard]] virtual Result<type::Ptr>
  typecheck(Environment &env) const noexcept = 0;
  [[nodiscard]] virtual Result<ast::Ptr>
  evaluate(Environment &env) noexcept = 0;
};

inline auto operator<<(std::ostream &out, ast::Ptr const &ast) noexcept
    -> std::ostream & {
  ast->print(out);
  return out;
}
} // namespace ast
} // namespace mint
