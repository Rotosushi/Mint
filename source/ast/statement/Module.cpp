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
#include "ast/statement/Module.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Result<type::Ptr> Module::typecheck(Environment &env) const noexcept {
  env.pushScope(m_name);

  for (auto &expresssion : m_expressions) {
    auto result = expresssion->typecheck(env);
    if (!result) {
      env.unbindScope(m_name);
      env.popScope();
      return result;
    }
  }

  env.popScope();
  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Module::evaluate(Environment &env) noexcept {
  env.pushScope(m_name);

  for (auto &expresssion : m_expressions) {
    auto result = expresssion->evaluate(env);
    if (!result) {
      env.unbindScope(m_name);
      env.popScope();
      return result;
    }
  }

  env.popScope();
  return env.getNilAst({}, {});
}
} // namespace ast
} // namespace mint
