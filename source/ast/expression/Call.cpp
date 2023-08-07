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
#include "ast/expression/Call.hpp"
#include "adt/Environment.hpp"
#include "ast/value/Lambda.hpp"
#include "type/composite/Lambda.hpp"

namespace mint {
namespace ast {
Call::Call(Attributes attributes, Location location, ast::Ptr callee,
           Arguments arguments) noexcept
    : Expression(Ast::Kind::Call, attributes, location),
      m_callee(std::move(callee)), m_arguments(std::move(arguments)) {}

auto Call::create(Attributes attributes, Location location, ast::Ptr callee,
                  Arguments arguments) noexcept -> ast::Ptr {
  return static_cast<ast::Ptr>(std::make_unique<Call>(
      attributes, location, std::move(callee), std::move(arguments)));
}

auto Call::classof(Ast const *ast) noexcept -> bool {
  return Ast::Kind::Call == ast->kind();
}

Ptr Call::clone_impl() const noexcept {
  Arguments cloned_arguments;
  cloned_arguments.reserve(m_arguments.size());
  for (auto &arg : m_arguments)
    cloned_arguments.emplace_back(arg->clone());

  return create(attributes(), location(), m_callee->clone(),
                std::move(cloned_arguments));
}

void Call::print(std::ostream &out) const noexcept {
  out << m_callee << " (";

  auto index = 0U;
  auto size = m_arguments.size();
  for (auto &arg : m_arguments) {
    arg->print(out);

    if (index++ < (size - 1))
      out << ", ";
  }

  out << ")";
}

// #NOTE: the type of a call expression is the result type of the
// callee, if and only if the type of each actual argument
// is equal to the type of each formal argument.
Result<type::Ptr> Call::typecheck(Environment &env) const noexcept {
  auto callee_result = m_callee->typecheck(env);
  if (!callee_result)
    return callee_result;

  auto callee_type = llvm::dyn_cast<type::Lambda>(callee_result.value());
  if (callee_type == nullptr) {
    std::stringstream message;
    message << m_callee;
    return {Error::Kind::CannotCallObject, m_callee->location(),
            message.view()};
  }

  auto &callee_arguments = callee_type->arguments();
  if (callee_arguments.size() != m_arguments.size()) {
    std::stringstream message;
    message << "Expected [" << m_arguments.size() << "] arguments, received ["
            << callee_arguments.size() << "] arguments";
    return {Error::Kind::ArgumentNumberMismatch, location(), message.view()};
  }

  auto callee_cursor = callee_arguments.begin();
  for (auto &argument : m_arguments) {
    auto argument_result = argument->typecheck(env);
    if (!argument_result)
      return argument_result;
    auto argument_type = argument_result.value();
    auto callee_argument_type = *callee_cursor;

    if (!argument_type->equals(callee_argument_type)) {
      std::stringstream message;
      message << "Expected type [" << callee_argument_type
              << "], received type [" << argument_type << "]";
      return {Error::Kind::ArgumentTypeMismatch, argument->location(),
              message.view()};
    }

    ++callee_cursor;
  }

  return cachedType(callee_type->result_type());
}

// #NOTE: evaluate the callee down to a lambda,
// set up the eval environment, eval the body
// within the eval environment, return the result.
Result<ast::Ptr> Call::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before we evaluate.
  MINT_ASSERT(cachedTypeOrAssert());
  // evaluate the callee down to it's lambda
  auto callee_result = m_callee->evaluate(env);
  if (!callee_result)
    return callee_result;

  // #NOTE: we assert here, because the only callable values
  // are lambdas, and since this call passed typechecking
  // we assume the existance of a lambda type of the callee
  // predicts a lambda value of the callee.
  auto callee = llvm::dyn_cast<ast::Lambda>(callee_result.value().get());
  MINT_ASSERT(callee != nullptr);

  // evaluate the actual arguments down to their values
  std::vector<ast::Ptr> actual_arguments;
  actual_arguments.reserve(m_arguments.size());
  for (auto &argument : m_arguments) {
    auto result = argument->evaluate(env);
    if (!result)
      return result;

    actual_arguments.emplace_back(result.value());
  }

  // inject the arguments into the lambda evaluation scope
  env.pushScope();
  auto actual_arguments_cursor = actual_arguments.begin();
  auto &callee_arguments = callee->arguments();
  for (auto &argument : callee_arguments) {
    auto bound =
        env.partialBindName(argument.name, argument.attributes, argument.type);
    if (!bound)
      return bound.error();
    auto binding = bound.value();
    auto &actual_argument = *actual_arguments_cursor;

    binding.setComptimeValue(actual_argument->clone());

    ++actual_arguments_cursor;
  }

  // evaluate the body in the lambda evaluation scope
  auto result = callee->body()->evaluate(env);
  // cleanup and return.
  // #NOTE: we don't need to check for success here,
  // as either way all we need to do is popScope
  // and return the result.
  env.popScope();
  return result;
}

// emit the call instruction after emitting each argument,
// return the call instruction as the result, as this
// represents the return value.
Result<llvm::Value *> Call::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before we evaluate.
  MINT_ASSERT(cachedTypeOrAssert());
  // #NOTE: calls must be codegen'ed within a basic block.
  MINT_ASSERT(env.hasInsertionPoint());
  // codegen the callee down to it's llvm::Function pointer
  auto callee_result = m_callee->codegen(env);
  if (!callee_result)
    return callee_result;

  // #NOTE: we assert here, because we assume that the callee is
  // a lambda, (or a variable bound to a lambda), and a lambda
  // is represented by a llvm::Function. (because, for now, all
  // lambdas are captureless.)
  auto function = llvm::dyn_cast<llvm::Function>(callee_result.value());
  MINT_ASSERT(function != nullptr);

  std::vector<llvm::Value *> actual_arguments;
  actual_arguments.reserve(m_arguments.size());
  for (auto &argument : m_arguments) {
    auto result = argument->codegen(env);
    if (!result) {
      env.popScope();
      return result;
    }

    actual_arguments.emplace_back(result.value());
  }

  return env.createLLVMCall(function, actual_arguments);
}
} // namespace ast
} // namespace mint
