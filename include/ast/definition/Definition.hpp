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

#include "adt/Identifier.hpp"
#include "ast/Ast.hpp"

namespace mint {
namespace ast {
/*
  #NOTE: the common base class of all Ast's which
  define a new name within the program.

  #NOTE: theoretically speaking, Definitions are
  Statements. That is, Definitions are Expressions
  which are only evaluated for their side effects.
  however it didn't seem necessary to make the inheritance
  tree model that. (though it wouldn't hurt anything to do so.)

  #NOTE: technically, this is the only abstract interior
  node which we need to have such that we can ask questions
  of any given definition within the program, in order to
  solve the problem of use-before-def given definitions more
  generally, instead of one at a time. theoretically speaking
  the use-before-def solution only relies on the aspects of
  definitions defined within the definition class, and not on
  any particular definition.
  and I didn't need to add the other interior nodes.
  it seemed like a good idea more for organization reasons.
  as it splits up the many kinds of nodes in the Ast into
  separate folders. (though that could have been done without
  the more complex inheritance heirarchy.)
  the one benefiet is that some virtual methods are a bit simpler
  to implement if they don't need to make use of sepcific Asts.
*/
class Definition : public Ast {
  mutable Error::Data m_use_before_def;
  std::optional<type::Ptr> m_annotation;
  Identifier m_name;

protected:
  Definition(Ast::Kind kind, Attributes attributes, Location location,
             std::optional<type::Ptr> annotation, Identifier name) noexcept
      : Ast{kind, attributes, location}, m_annotation{annotation},
        m_name{name} {}

  virtual Ptr clone_impl() const noexcept = 0;
  virtual ir::detail::Parameter flatten_impl(ir::Mir &ir) const noexcept = 0;

public:
  ~Definition() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return (ast->kind() >= Ast::Kind::Definition) &&
           (ast->kind() <= Ast::Kind::EndDefinition);
  }

  bool isUseBeforeDef() const noexcept {
    return std::holds_alternative<Error::UseBeforeDef>(m_use_before_def);
  }
  auto getUseBeforeDef() const noexcept {
    return std::get<Error::UseBeforeDef>(m_use_before_def);
  }
  void setUseBeforeDef(Error::UseBeforeDef const &usedef) const noexcept {
    m_use_before_def = usedef;
  }
  void clearUseBeforeDef() const noexcept {
    m_use_before_def = std::monostate{};
  }
  std::optional<type::Ptr> annotation() const noexcept { return m_annotation; }
  Identifier name() const noexcept { return m_name; }

  std::optional<Identifier> getDefinitionName() const noexcept override {
    return m_name;
  }

  virtual std::optional<Error>
  checkUseBeforeDef(Error::UseBeforeDef &ubd) const noexcept = 0;

  virtual void print(std::ostream &out) const noexcept = 0;

  virtual Result<type::Ptr> typecheck(Environment &env) const noexcept = 0;
  virtual Result<ast::Ptr> evaluate(Environment &env) noexcept = 0;
  virtual Result<llvm::Value *> codegen(Environment &) noexcept = 0;
};
} // namespace ast
} // namespace mint
