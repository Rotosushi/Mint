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
Result<type::Ptr> Boolean::typecheck(Environment &env) const noexcept {
  setCachedType(env.getBooleanType());
  return env.getBooleanType();
}

// we don't really need to 'evaluate' scalar values.
// however, we must be able to return a valid scalar
// value from calling 'evaluate' on a scalar value.
// since we manage Ast objects by shared_ptrs,
// we must return a valid std::shared_ptr to the same
// scalar value we are currently 'evaluating'
// this forces us to perform a memory allocation here.
// even though theoretically this isn't required.
// we must either return a new Scalar value object within
// a new shared_ptr control block, or we can use
// shared_from_this to return a shared_ptr to the
// same control block. either way, this is inefficient in
// the long run.
Result<ast::Ptr> Boolean::evaluate([[maybe_unused]] Environment &env) noexcept {
  return shared_from_this();
}

Result<llvm::Value *> Boolean::codegen(Environment &env) noexcept {
  return env.getLLVMBoolean(m_value);
}
} // namespace ast
} // namespace mint
