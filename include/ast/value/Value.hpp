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
#include "ir/detail/Parameter.hpp"

namespace mint {
namespace ast {
//   #NOTE: the common base class of all Ast's which
//   represent some value within the program.
class Value : public Ast {
protected:
  Value(Ast::Kind kind, Attributes attributes, Location location) noexcept
      : Ast{kind, attributes, location} {}

  [[nodiscard]] virtual Ptr clone_impl() const noexcept = 0;
  // #NOTE: we don't want to call flatten_impl on values,
  // because values are associated with a parameter.
  virtual void flatten(ir::Mir &ir) const noexcept = 0;
  virtual void
  flatten_immediate(ir::detail::Parameter &parameter) const noexcept = 0;

public:
  ~Value() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return (ast->kind() >= Ast::Kind::Value) &&
           (ast->kind() <= Ast::Kind::EndValue);
  }

  auto isImmediate() const noexcept -> bool {
    return (kind() == Ast::Kind::Nil) || (kind() == Ast::Kind::Boolean) ||
           (kind() == Ast::Kind::Integer) || (kind() == Ast::Kind::Variable);
  }

  virtual void print(std::ostream &out) const noexcept = 0;

  std::optional<Identifier> getDefinitionName() const noexcept override {
    if (isRoot()) {
      auto prev = prevAst();
      return prev->getDefinitionName();
    }
    return std::nullopt;
  }

  virtual Result<type::Ptr> typecheck(Environment &env) const noexcept = 0;
  // #TODO: values are such because they are not evaluated,
  // they are what is evaluated with. is there some way of
  // not performing a clone to return a valid ast::Ptr?
  // my first thought is with a shared_ptr.
  virtual Result<ast::Ptr> evaluate(Environment &env) noexcept = 0;
  virtual Result<llvm::Value *> codegen(Environment &) noexcept = 0;
};
} // namespace ast
} // namespace mint
