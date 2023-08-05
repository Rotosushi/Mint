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
Variable::Variable(Attributes attributes, Location location,
                   Identifier name) noexcept
    : Expression{Ast::Kind::Variable, attributes, location}, m_name{name} {}

[[nodiscard]] auto Variable::create(Attributes attributes, Location location,
                                    Identifier name) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Variable>(attributes, location, name));
}

auto Variable::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Variable;
}

Ptr Variable::clone() const noexcept {
  return create(attributes(), location(), m_name);
}

void Variable::print(std::ostream &out) const noexcept { out << m_name; }

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

  auto def = env.qualifyName(found.value());
  auto undef = m_name;

  return {Error::Kind::UseBeforeDef, UseBeforeDefNames{def, undef},
          env.localScope()};
}

/*
  #NOTE: the type of a variable is the type of the value it is bound to.

  #NOTE: iff the variable is not found and the variable appears within a
  definition, we report a use-before-definition error, to allow definitions
  to be declared in any order.
*/
Result<type::Ptr> Variable::typecheck(Environment &env) const noexcept {
  auto result = env.lookupBinding(m_name);
  if (!result)
    return handleUseBeforeDef(result.error(), env);

  return setCachedType(result.value().type());
}

Result<ast::Ptr> Variable::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  auto result = env.lookupBinding(m_name);
  MINT_ASSERT(result.hasValue());
  auto binding = result.value();

  // we cannot evaluate a variable given a
  // binding without a comptime value
  if (!binding.hasComptimeValue())
    return handleUseBeforeDef(env);

  return binding.comptimeValueOrAssert();
}

// #NOTE: variables don't -need- to be codegen'ed within a basic_block.
// we do need to handle the difference in code however.
Result<llvm::Value *> Variable::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  auto result = env.lookupBinding(m_name);
  MINT_ASSERT(result.hasValue());
  auto binding = result.value();

  // we cannot codegen a variable given a
  // binding without a runtime value
  if (!binding.hasRuntimeValue())
    return handleUseBeforeDef(env);

  auto type = binding.type()->toLLVM(env);
  auto value = binding.runtimeValueOrAssert();

  if (!env.hasInsertionPoint()) {
    if (auto global = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
      return global->getInitializer();
    }

    abort("Global access without an InsertPoint");
  } else {
    // #NOTE: #RULE runtime variables are stored
    // in memory so we need to load the value,
    // such that it can be used in expressions.
    return env.createLLVMLoad(type, value);
  }
}
} // namespace ast
} // namespace mint