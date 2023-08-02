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
#include "ast/value/Nil.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Nil::Nil(Attributes attributes, Location location) noexcept
    : Value{Ast::Kind::Nil, attributes, location} {}

[[nodiscard]] auto Nil::create(Attributes attributes,
                               Location location) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Nil>(attributes, location));
}

auto Nil::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Nil;
}

Ptr Nil::clone() const noexcept { return create(attributes(), location()); }

void Nil::print(std::ostream &out) const noexcept { out << "nil"; }

Result<type::Ptr> Nil::typecheck(Environment &env) const noexcept {
  setCachedType(env.getNilType());
  return env.getNilType();
}

/*
  #NOTE: we don't need to do anything to evaluate a scalar value,
  however the type signature forces us to return a ast::Ptr,
  meaning we have to return a clone of the scalar value.
  this is not very efficient. a different evaluation strategy
  might be better suited to interpretation. over evaluating
  asts directly. because it is unessesary to clone here,
  theoretically speaking.
*/
Result<ast::Ptr> Nil::evaluate([[maybe_unused]] Environment &env) noexcept {
  return clone();
}

Result<llvm::Value *> Nil::codegen(Environment &env) noexcept {
  return env.getLLVMNil();
}

} // namespace ast
} // namespace mint
