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
class Definition : public Ast {
  std::optional<type::Ptr> m_annotation;
  Identifier m_name;

protected:
  Definition(Ast::Kind kind, Attributes attributes, Location location,
             std::optional<type::Ptr> annotation, Identifier name) noexcept
      : Ast{kind, attributes, location}, m_annotation{annotation},
        m_name{name} {}

public:
  static auto classof(Ast const *ast) noexcept -> bool {
    return (ast->kind() >= Ast::Kind::Definition) &&
           (ast->kind() <= Ast::Kind::EndDefinition);
  }

  std::optional<type::Ptr> annotation() const noexcept { return m_annotation; }
  Identifier name() const noexcept { return m_name; }

  virtual Ptr clone(Allocator &allocator) const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;

  virtual Result<type::Ptr> typecheck(Environment &env) const noexcept = 0;
  virtual Result<ast::Ptr> evaluate(Environment &env) noexcept = 0;
};
} // namespace ast
} // namespace mint
