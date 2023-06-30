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

#include "ast/semantics/Semantics.hpp"
#include "scan/Token.hpp"

namespace mint {
namespace ast {
class Binop : public Semantics {
  Token m_op;
  Ptr m_left;
  Ptr m_right;

public:
  Binop(Attributes attributes, Location location, Token op, Ptr left,
        Ptr right) noexcept
      : Semantics{Ast::Kind::Binop, attributes, location}, m_op{op},
        m_left{std::move(left)}, m_right{std::move(right)} {}
  ~Binop() noexcept override = default;

  static auto create(Allocator &allocator, Attributes attributes,
                     Location location, Token op, Ptr left, Ptr right) noexcept
      -> Ptr {
    return std::allocate_shared<Binop, Allocator>(
        allocator, attributes, location, op, std::move(left), std::move(right));
  }

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Binop;
  }

  Ptr clone(Allocator &allocator) const noexcept override {
    return create(allocator, attributes(), location(), m_op,
                  m_left->clone(allocator), m_right->clone(allocator));
  }

  void print(std::ostream &out) const noexcept override {
    out << m_left << " " << m_op << " " << m_right;
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
