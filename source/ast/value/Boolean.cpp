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
#include "ast/value/Boolean.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Ptr Boolean::clone(Environment &env) const noexcept {
  return env.getBooleanAst(attributes(), location(), m_value);
}

Result<type::Ptr> Boolean::typecheck(Environment &env) const noexcept {
  setCachedType(env.getBooleanType());
  return env.getBooleanType();
}

// we don't really need to 'evaluate' scalar values.
// however, we must be able to return a valid scalar
// value from calling 'evaluate' on a scalar value.
// since we manage Ast objects by an allocation list,
// we can simply return this.
Result<ast::Ptr> Boolean::evaluate([[maybe_unused]] Environment &env) noexcept {
  return this;
}

Result<llvm::Value *> Boolean::codegen(Environment &env) noexcept {
  return env.getLLVMBoolean(m_value);
}
} // namespace ast
} // namespace mint
