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

  return handleUseBeforeDef(env);
}

auto Variable::handleUseBeforeDef(Environment &env) const noexcept -> Error {
  auto found = getDefinitionName();
  if (!found) {
    // We cannot delay expressions generally, only definitions.
    return {Error::Kind::NameUnboundInScope, location(), m_name.view()};
  }

  auto def = env.getQualifiedName(found.value());
  // #NOTE: if the undef name is globally qualified, we
  // assume the programmer specified the correct name of
  // the use-before-def. otherwise we assume that the
  // undef name refers to a binding that is going to be
  // created within the scope local to this definition.
  auto undef =
      m_name.isGloballyQualified() ? m_name : env.getQualifiedName(m_name);

  // does it work to call bindUseBeforeDef here instead of when we
  // ignore the UseBeforeDef Error during the loop?
  // not easily, we need a shared_ptr to the definition, which
  // requires walking up the Ast to the top.
  return Error{Error::Kind::UseBeforeDef, def, undef, env.localScope()};
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

Result<llvm::Value *> Variable::codegen(Environment &env) noexcept {
  
}
} // namespace ast
} // namespace mint