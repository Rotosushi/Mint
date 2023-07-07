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
auto Variable::handleUseBeforeDef(Error &error, Environment &env) const noexcept
    -> Error {
  // if this is not a use-before-def variable
  if (error.kind() != Error::Kind::NameUnboundInScope) {
    return {error.kind(), location(), m_name.view()};
  }

  auto found = getDefinitionName();
  if (!found) {
    // this variable is not use-before-def within a definition,
    // so it's an attempt to use a use-before-def variable
    // as a value, which is an error.
    return {error.kind(), location(), m_name.view()};
  }

  /*
    #NOTE: we need to map the use-before-def definition to
    an Identifier, that Identifier is going to be used to
    lookup this use-before-def later, when it is defined.
    thus it makes sense to fully qualify both names,
    as when we declare a new definition, we define the fully
    qualified name, and thus that fully qualified name is
    looked up in the map finding the definition which relies
    on that name.
  */
  auto def = found.value();
  auto q_def = env.getQualifiedName(def);
  auto undef = m_name;
  auto q_undef = env.getQualifiedName(undef);
  return Error{Error::Kind::UseBeforeDef, def, q_def, undef, q_undef};
}

auto Variable::handleUseBeforeDef(Environment &env) const noexcept -> Error {
  auto found = getDefinitionName();
  if (!found) {
    // this variable is not use-before-def within a definition,
    // so it's an attempt to use a use-before-def variable
    // as a value, which is an error.
    return {Error::Kind::NameUnboundInScope, location(), m_name.view()};
  }

  /*
    #NOTE: we need to map the use-before-def definition to
    an Identifier, that Identifier is going to be used to
    lookup this use-before-def later, when it is defined.
    thus it makes sense to fully qualify both names,
    as when we declare a new definition, we define the fully
    qualified name, and thus that fully qualified name is
    looked up in the map finding the definition which relies
    on that name.
  */
  auto def = found.value();
  auto q_def = env.getQualifiedName(def);
  auto undef = m_name;
  auto q_undef = env.getQualifiedName(undef);
  return Error{Error::Kind::UseBeforeDef, def, q_def, undef, q_undef};
}

/*
  #NOTE: the type of a variable is the type of the value it is bound to.

  #NOTE: iff the variable is not found and the variable appears within a
  definition, we report a use-before-definition error, to allow definitions
  to be declared in any order.
*/
Result<type::Ptr> Variable::typecheck(Environment &env) const noexcept {
  auto bound = env.lookupBinding(m_name);
  if (!bound)
    return handleUseBeforeDef(bound.error(), env);

  auto result = bound.value().type();
  setCachedType(result);
  return result;
}

Result<ast::Ptr> Variable::evaluate(Environment &env) noexcept {
  auto bound = env.lookupBinding(m_name);
  if (!bound)
    return handleUseBeforeDef(bound.error(), env);

  // we cannot evaluate a variable given a
  // partial binding.
  if (bound.value().isPartial()) {
    return handleUseBeforeDef(env);
  }

  return bound.value().value();
}
} // namespace ast
} // namespace mint