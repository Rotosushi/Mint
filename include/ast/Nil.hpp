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

#include "ast/Value.hpp"

namespace mint {
class NilAst : public ValueAst {
public:
  NilAst(Attributes attributes, Location location) noexcept
      : ValueAst{Ast::Kind::Nil, attributes, location} {}

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Nil;
  }

  Ptr clone(Allocator &allocator) const noexcept override {
    return std::allocate_shared<NilAst>(allocator, attributes(), location());
  }

  void print(std::ostream &out) const noexcept override {
    out << "nil";
  }

  Result<Type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<Ast::Ptr> evaluate(Environment &env) noexcept override;
};
} // namespace mint
