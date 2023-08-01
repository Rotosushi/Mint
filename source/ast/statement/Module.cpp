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
Ptr Module::clone(Environment &env) const noexcept {
  Expressions expressions;
  for (auto &expression : m_expressions) {
    expressions.emplace_back(expression->clone(env));
  }

  return env.getModuleAst(attributes(), location(), m_name,
                          std::move(expressions));
}

Result<type::Ptr> Module::typecheck(Environment &env) const noexcept {
  env.pushScope(m_name);

  for (auto &expression : m_expressions) {
    auto result = expression->typecheck(env);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        env.unbindScope(m_name);
        env.popScope();
        return result;
      }

      // all we need to do to handle use-before-def at this point
      // is attempt to bind this use-before-def term to the map.
      // #NOTE: we clone here, because we don't want to leave the
      // module itself in an undefined state. if we move, then
      // the next time we iterate we will run into a null unique-ptr
      // at this definition. 
      if (auto failed = env.bindUseBeforeDef(error, expression->clone(env))) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
    }
  }

  env.popScope();
  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Module::evaluate(Environment &env) noexcept {
  env.pushScope(m_name);

  for (auto &expression : m_expressions) {
    auto result = expression->evaluate(env);
    if (!result) {
      auto &error = result.error();

      if (!error.isUseBeforeDef()) {
        env.unbindScope(m_name);
        env.popScope();
        return result;
      }

      if (auto failed = env.bindUseBeforeDef(error, expression->clone(env))) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
    }
  }

  env.popScope();
  return env.getNilAst({}, {});
}

Result<llvm::Value *> Module::codegen(Environment &env) noexcept {
  env.pushScope(m_name);

  for (auto &expression : m_expressions) {
    auto result = expression->codegen(env);
    if (!result) {
      auto &error = result.error();

      if (!error.isUseBeforeDef()) {
        env.unbindScope(m_name);
        env.popScope();
        return result;
      }

      if (auto failed = env.bindUseBeforeDef(error, expression->clone(env))) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
    }
  }

  env.popScope();
  return env.getLLVMNil();
}
} // namespace ast
} // namespace mint
