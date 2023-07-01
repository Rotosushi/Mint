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
#include "ast/expression/Variable.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Result<type::Ptr> Variable::typecheck(Environment &env) const noexcept {
  auto bound = env.lookup(m_name);
  if (!bound) {
    auto &error = bound.error();
    // if this is a use-before-def variable
    if (error.kind() == Error::Kind::NameUnboundInScope) {
      auto found = getDefinitionName();
      if (found) {
        auto def = found.value();
        auto q_def = env.getQualifiedName(def);
        auto undef = m_name;
        auto q_undef = env.getQualifiedName(undef);
        return Error{Error::Kind::UseBeforeDef, def, q_def, undef, q_undef};
      }
      // else #NOTE:
      // this variable is not use-before-def within a definition,
      // so it's an attempt to use a use-before-def variable
      // as a value, which is an error.
    }
    // #NOTE: the Error generated within the env does not
    // have the location information, so we construct a new
    // Error here adding in more context.
    return {error.kind(), location(), m_name.view()};
  }

  auto result = bound.value().type();
  setCachedType(result);
  return result;
}

Result<ast::Ptr> Variable::evaluate(Environment &env) noexcept {
  auto bound = env.lookup(m_name);
  if (!bound) {
    return {bound.error().kind(), location(), m_name.view()};
  }

  return bound.value().value();
}
} // namespace ast
} // namespace mint