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
#include "ast/value/Nil.hpp"
#include "ir/Instruction.hpp"

namespace mint {
namespace ast {
Module::Module(Attributes attributes, Location location, Identifier name,
               Expressions expressions) noexcept
    : Statement{Ast::Kind::Module, attributes, location}, m_name{name},
      m_expressions{std::move(expressions)} {
  for (auto &expression : m_expressions)
    expression->prevAst(this);
}

[[nodiscard]] auto Module::create(Attributes attributes, Location location,
                                  Identifier name,
                                  Expressions expressions) noexcept
    -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(std::make_unique<Module>(
      attributes, location, name, std::move(expressions)));
}

auto Module::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Module;
}

Ptr Module::clone_impl() const noexcept {
  Expressions expressions;
  for (auto &expression : m_expressions) {
    expressions.emplace_back(expression->clone());
  }

  return create(attributes(), location(), m_name, std::move(expressions));
}

ir::detail::Parameter Module::flatten_impl(ir::Mir &ir) const noexcept {
  //  construct an Mir object to represent
  //  instructions within the module
  ir::Mir expressions;
  expressions.reserve(m_expressions.size());
  for (auto &expression : m_expressions) {
    expression->flatten_impl(expressions);
  }
  // insert the module instruction into the current Mir object
  auto pair = ir.emplace_back<ir::Module>(m_name, std::move(expressions));
  return pair.first;
}

void Module::print(std::ostream &out) const noexcept {
  out << "module " << m_name << " { \n";

  for (auto &expression : m_expressions)
    out << expression << "\n";

  out << "}";
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

      if (auto failed = env.bindUseBeforeDef(error, expression)) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
    }
  }

  env.popScope();

  return cachedType(env.getNilType());
}

Result<ast::Ptr> Module::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
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

      if (auto failed = env.bindUseBeforeDef(error, expression)) {
        env.unbindScope(m_name);
        env.popScope();
        return failed.value();
      }
    }
  }

  env.popScope();
  return ast::Nil::create({}, {});
}

Result<llvm::Value *> Module::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
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

      if (auto failed = env.bindUseBeforeDef(error, expression)) {
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
