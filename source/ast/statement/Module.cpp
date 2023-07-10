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

  /*
    #NOTE: if a name fails to typecheck because of a use-before-def error,
      what steps do we need to take here?

      well, in order to properly handle use-before-def between definitions
      within the module, use-before-def cannot stop the typechecking of a
      module. therefore we must continue to typecheck expressions if
      we get a use-before-def.
      however, we need to construct a partial definition of the use-before-def
      when we resolve it and not a full definition, because we simply do not
    have the ability to compute the value of this use-before-def at this point
    in time. (if we did, we would call evaluate twice on the same definition
      within the same scope, which would generate a name conflict.)
  */
  for (auto &expression : m_expressions) {
    auto result = expression->typecheck(env);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        env.unbindScope(m_name);
        env.popScope();
        return result;
      }

      if (auto failed = env.bindUseBeforeDef(error, expression)) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
      // else ... we continue to typecheck expressions within the
      // module. any definitions which we typecheck will create
      // partial bindings, and then will partially resolve any
      // use-before-def terms that rely on those definitions.
    }
  }

  env.popScope();
  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Module::evaluate(Environment &env) noexcept {
  env.pushScope(m_name);

  /*
    #NOTE: if we had some use-before-def definitions occur within this
    module while we were typechecking, then we created partial-bindings
    of those terms when we typecheckd the dependant definitions.
    so now when we evaluate those terms we will retrieve a partial binding
  */
  for (auto &expression : m_expressions) {
    auto result = expression->evaluate(env);
    if (!result) {
      auto &error = result.error();

      if (!error.isUseBeforeDef()) {
        env.unbindScope(m_name);
        env.popScope();
        return result;
      }

      if (auto failed = env.bindUseBeforeDef(error, expression)) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
      // else we leave the definition to be defined later, when
      // we define the use-before-def variable. any definitions
      // which we evaluate will create full definitions,
      // and then we resolve any use-before-def defintions
      // which depend upon that definition.
    }
  }

  env.popScope();
  return env.getNilAst({}, {});
}
} // namespace ast
} // namespace mint
