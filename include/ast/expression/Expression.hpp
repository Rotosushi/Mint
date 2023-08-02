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
/*
  The base class of all Ast's which evaluate to some
  usable value.
*/
class Expression : public Ast {
protected:
  Expression(Ast::Kind kind, Attributes attributes, Location location) noexcept
      : Ast{kind, attributes, location} {}

public:
  ~Expression() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return (ast->kind() >= Ast::Kind::Expression) &&
           (ast->kind() <= Ast::Kind::EndExpression);
  }

  virtual Ptr clone() const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;

  std::optional<Identifier> getDefinitionName() const noexcept override {
    if (havePrevAst()) {
      auto prev = getPrevAst();
      return prev->getDefinitionName();
    }
    return std::nullopt;
  }

  virtual Result<type::Ptr> typecheck(Environment &env) const noexcept = 0;
  virtual Result<ast::Ptr> evaluate(Environment &env) noexcept = 0;
  virtual Result<llvm::Value *> codegen(Environment &) noexcept = 0;
};
} // namespace ast
} // namespace mint
