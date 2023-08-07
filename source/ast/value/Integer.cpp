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
#include "ast/value/Integer.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Integer::Integer(Attributes attributes, Location location, int value) noexcept
    : Value{Ast::Kind::Integer, attributes, location}, m_value{value} {}

auto Integer::value() const noexcept -> int { return m_value; }

[[nodiscard]] auto Integer::create(Attributes attributes, Location location,
                                   int value) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Integer>(attributes, location, value));
}

auto Integer::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Integer;
}

void Integer::print(std::ostream &out) const noexcept { out << m_value; }

Ptr Integer::clone_impl() const noexcept {
  return create(attributes(), location(), m_value);
}

Result<type::Ptr> Integer::typecheck(Environment &env) const noexcept {
  return cachedType(env.getIntegerType());
}

Result<ast::Ptr> Integer::evaluate([[maybe_unused]] Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  return shared_from_this();
}

Result<llvm::Value *> Integer::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  return env.getLLVMInteger(m_value);
}
} // namespace ast
} // namespace mint
