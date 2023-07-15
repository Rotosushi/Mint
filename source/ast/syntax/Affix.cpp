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
#include "ast/syntax/Affix.hpp"

namespace mint {
namespace ast {
Result<type::Ptr> Affix::typecheck(Environment &env) const noexcept {
  auto result = m_ast->typecheck(env);
  if (!result)
    return result;

  setCachedType(result.value());
  return result;
}

Result<ast::Ptr> Affix::evaluate(Environment &env) noexcept {
  return m_ast->evaluate(env);
}

Result<llvm::Value *> Affix::codegen(Environment &env) noexcept {
  return m_ast->codegen(env);
}
} // namespace ast
} // namespace mint
