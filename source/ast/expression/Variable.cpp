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

  return Error{Error::Kind::UseBeforeDef, UseBeforeDefNames{def, undef},
               env.localScope()};
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

  return setCachedType(bound.value().type());
}

Result<ast::Ptr> Variable::evaluate(Environment &env) noexcept {
  auto bound = env.lookupBinding(m_name);
  if (!bound)
    return Result<ast::Ptr>{handleUseBeforeDef(bound.error(), env)};

  auto binding = bound.value();

  // we cannot evaluate a variable given a
  // binding without a comptime value
  if (!binding.hasComptimeValue())
    return Result<ast::Ptr>{handleUseBeforeDef(env)};

  return Result<ast::Ptr>{binding.comptimeValueOrAssert()->clone()};
}

Result<llvm::Value *> Variable::codegen(Environment &env) noexcept {
  auto bound = env.lookupBinding(m_name);
  if (!bound)
    return Result<llvm::Value *>{handleUseBeforeDef(bound.error(), env)};
  auto binding = bound.value();

  // we cannot codegen a variable given a
  // binding without a runtime value
  if (!binding.hasRuntimeValue())
    return Result<llvm::Value *>{handleUseBeforeDef(env)};

  auto type = binding.type()->toLLVM(env);
  auto value = binding.runtimeValueOrAssert();

  // #TODO: this only makes sense when codegening
  // in a global context. as otherwise we want to
  // load/store the global variable itself.
  // however there are no local contexts yet. so
  // this check is fine for now.
  if (auto global = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
    return Result<llvm::Value *>{global->getInitializer()};
  } else {
    // #NOTE: given that runtime variables are stored
    // in memory we need to load the value,
    // such that it can be used in expressions.
    return Result<llvm::Value *>{env.createLLVMLoad(type, value)};
  }
}
} // namespace ast
} // namespace mint