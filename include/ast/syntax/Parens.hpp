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

#include "ast/Ast.hpp"

namespace mint {
namespace ast {
class Parens : public Ast {
  Ptr m_ast;

public:
  Parens(Attributes attributes, Location location, Ptr ast) noexcept
      : Ast{Ast::Kind::Parens, attributes, location}, m_ast{std::move(ast)} {}
  ~Parens() noexcept override = default;

  static auto create(Allocator &allocator, Attributes attributes,
                     Location location, Ptr ast) noexcept -> Ptr {
    return std::allocate_shared<Parens, Allocator>(allocator, attributes,
                                                   location, std::move(ast));
  }

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Parens;
  }

  Ptr clone(Allocator &allocator) const noexcept override {
    return create(allocator, attributes(), location(), m_ast->clone(allocator));
  }

  void print(std::ostream &out) const noexcept override {
    out << "(" << m_ast << ")";
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
