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

#include "llvm/IR/Value.h"

namespace mint {
class Environment;

namespace ast {
class Ast;

/*
  what are the advantages of
  using Ptr = std::unique_ptr<Ast>;
  instead?

  overall the differences are minimal,
  so I am unable to say one is a better
  choice as of right now.

  either way we need and want a clone method.
  (with unique_ptr it is necessary in all cases
  that the data within a given Ast is needed across
  multiple Ast's, with shared_ptr it is not necessary
  in all cases, but still needed in specific circumstances.
  such as declaring a new variable with the same value as
  another existing variable. a circumstance where we could
  avoid a clone is reference semantics within
  the interpreter, as two shared_ptrs could literally
  refer to the same value allocation.
  however, just as easily we can simply construct a
  reference ast::Value which points to another value
  and store that in a unique_ptr. so again, the
  differences between these two are minimal)

  unique_ptr is strict about enforcing move semantics,
  which is more efficient from a speed perspective,
  and with no control block it is more efficient from
  a memory perspective. additionally, there is no need
  for atomic instructions, which are another potential
  speed bottleneck. However this implementation of the
  language is not intended to be the fastest (yet).
  as currently the goal is correctness, and new features.

  either way, data synchronization is manual.
  (atomic increment only synchronizes the
  sharing itself accross threads, not access to
  the stored data.)

  I'd say that the biggest advantage shared_ptr has
  as of right now during development is that it has
  copy-semantics. which makes implementing an interpreter
  with shared_ptrs slightly easier to get right.
  but I could also argue that enforced move semantics
  would force the implementation to be aware
  of what part of the interpreter owns what, and that
  could be more efficient.
  the issue there is that the goal is currently
  correctness and not performance.
  so shared_ptr is what we are going with for the
  forseeable future.

  #NOTE(7/15/23): it might be best to disconnect the allocation
  from the tree entirely. storing all smart pointers in a separate
  structure, and constructing the tree itself out of raw pointers.
*/
using Ptr = Ast *;

class Ast {
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

  [[nodiscard]] virtual Ptr clone(Environment &env) const noexcept = 0;
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
};

inline auto operator<<(std::ostream &out, ast::Ptr const &ast) noexcept
    -> std::ostream & {
  ast->print(out);
  return out;
}
} // namespace ast
} // namespace mint
