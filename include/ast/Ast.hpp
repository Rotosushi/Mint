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
#include "adt/Result.hpp"
#include "ir/Mir.hpp"
#include "scan/Location.hpp"
#include "type/Type.hpp"
#include "utility/Assert.hpp"

#include "llvm/IR/Value.h"

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
    Function,
    EndDefinition,

    // Values
    Value,
    Nil,
    Boolean,
    Integer,
    Lambda,
    EndValue,

    // Syntax
    Syntax,
    Affix,
    Parens,
    EndSyntax,

    // Expression
    Expression,
    Binop,
    Call,
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

  auto cachedType(type::Ptr type) const noexcept -> type::Ptr {
    return m_cached_type = type;
  }

  auto kind(Kind kind) noexcept { return m_kind = kind; }
  auto attributes(Attributes attributes) noexcept {
    return m_attributes = attributes;
  }
  auto location(Location location) noexcept { return m_location = location; }

  [[nodiscard]] virtual Ptr clone_impl() const noexcept = 0;
  virtual void flatten_impl(ir::Mir &ir) const noexcept = 0;

public:
  virtual ~Ast() noexcept = default;

  bool isRoot() const noexcept { return m_prev_ast != nullptr; }
  Ast *prevAst() const noexcept { return m_prev_ast; }
  Ast *prevAst(Ast *prev_ast) const noexcept { return m_prev_ast = prev_ast; }

  [[nodiscard]] auto cachedType() const noexcept { return m_cached_type; }
  [[nodiscard]] auto cachedTypeOrAssert() const noexcept {
    MINT_ASSERT(m_cached_type != nullptr);
    return m_cached_type;
  }
  [[nodiscard]] auto kind() const noexcept { return m_kind; }
  [[nodiscard]] auto attributes() const noexcept { return m_attributes; }
  [[nodiscard]] auto location() const noexcept { return m_location; }

  [[nodiscard]] auto clone() const noexcept -> Ptr {
    // #NOTE: we assert that every ast is typechecked
    // before it is evaluated or codegened.
    // because of this, a cloned ast would not be counted
    // as having been typechecked, due to clone being a
    // bare virtual fn call, the members of the base
    // class did not get cloned.
    Ptr clone = clone_impl();
    clone->prevAst(m_prev_ast);
    clone->cachedType(m_cached_type);
    clone->kind(m_kind);
    clone->attributes(m_attributes);
    clone->location(m_location);
    return clone;
  }

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
  [[nodiscard]] virtual Result<llvm::Value *>
  codegen(Environment &env) noexcept = 0;

  [[nodiscard]] ir::Mir flatten() const noexcept {
    ir::Mir result;
    flatten_impl(result);
    return result;
  }
};

inline auto operator<<(std::ostream &out, ast::Ptr const &ast) noexcept
    -> std::ostream & {
  ast->print(out);
  return out;
}
} // namespace ast
} // namespace mint
