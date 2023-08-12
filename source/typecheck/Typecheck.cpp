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
#include "typecheck/Typecheck.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"

namespace mint {
struct TypecheckScalar {
  Environment *env;

  TypecheckScalar(Environment &env) noexcept : env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Scalar &scalar) noexcept {
    return std::visit(*this, scalar.variant());
  }

  Result<type::Ptr> operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return env->getNilType();
  }

  Result<type::Ptr> operator()([[maybe_unused]] bool &boolean) noexcept {
    return env->getBooleanType();
  }

  Result<type::Ptr> operator()([[maybe_unused]] int &integer) noexcept {
    return env->getIntegerType();
  }
};

Result<type::Ptr> typecheck(ir::detail::Scalar &scalar,
                            Environment &env) noexcept {
  TypecheckScalar visitor(env);
  return visitor(scalar);
}

Result<type::Ptr> typecheck(ir::detail::Index &index, ir::Mir &ir,
                            Environment &env) noexcept;

struct TypecheckParameter {
  ir::Mir *ir;
  Environment *env;

  TypecheckParameter(ir::Mir &ir, Environment &env) noexcept
      : ir(&ir), env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Parameter &parameter) noexcept {
    return std::visit(*this, parameter.variant());
  }

  Result<type::Ptr> operator()(ir::detail::Scalar &scalar) noexcept {
    return typecheck(scalar, *env);
  }

  Result<type::Ptr> operator()(Identifier &variable) noexcept {
    auto result = env->lookupBinding(variable);
    if (!result)
      return result.error();
    // #TODO: handle use-before-def
    // #TODO: create a type cache
    return result.value().type();
  }

  Result<type::Ptr> operator()(ir::detail::Index &index) noexcept {
    return typecheck(index, *ir, *env);
  }
};

Result<type::Ptr> typecheck(ir::detail::Parameter &parameter, ir::Mir &ir,
                            Environment &env) noexcept {
  TypecheckParameter visitor(ir, env);
  return visitor(parameter);
}

struct TypecheckInstruction {
  ir::Mir *ir;
  Environment *env;

  TypecheckInstruction(ir::Mir &ir, Environment &env) noexcept
      : ir(&ir), env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Index &index) noexcept {
    return std::visit(*this, (*ir)[index].variant());
  }

  Result<type::Ptr> operator()(ir::detail::Scalar &scalar) noexcept {
    return typecheck(scalar, *env);
  }

  Result<type::Ptr> operator()(ir::Let &let) noexcept {
    auto found = env->lookupLocalBinding(let.name());
    if (found) // #TODO: better error messages
      return {Error::Kind::NameAlreadyBoundInScope};

    auto result = typecheck(let.parameter(), *ir, *env);
    if (!result)
      return result;

    // #TODO: handle optional type annotation
    // #TODO: handle bound variable attributes
    if (auto bound = env->declareName(let.name(), {}, result.value());
        bound.failure()) {
      return bound.error();
    }

    return result.value();
  }

  Result<type::Ptr> operator()(ir::Binop &binop) noexcept {
    auto overloads = env->lookupBinop(binop.op());
    if (!overloads)
      return {Error::Kind::UnknownBinop};

    auto left = typecheck(binop.left(), *ir, *env);
    if (!left)
      return left;

    auto right = typecheck(binop.right(), *ir, *env);
    if (!right)
      return right;

    auto instance = overloads->lookup(left.value(), right.value());
    if (!instance)
      return {Error::Kind::BinopTypeMismatch};

    return instance->result_type;
  }

  Result<type::Ptr> operator()(ir::Call &call) noexcept {
    auto callee = typecheck(call.callee(), *ir, *env);
    if (!callee)
      return callee;

    // #TODO: extend this to handle lambdas or functions.
    auto callee_type = llvm::dyn_cast<type::Lambda>(callee.value());
    if (callee_type == nullptr)
      return {Error::Kind::CannotCallObject};
    auto function_type = callee_type->function_type();

    auto &actual_arguments = call.arguments();
    auto &formal_arguments = function_type->arguments();
    if (formal_arguments.size() != actual_arguments.size())
      return {Error::Kind::ArgumentNumberMismatch};

    auto cursor = formal_arguments.begin();
    for (auto &actual_argument : actual_arguments) {
      auto result = typecheck(actual_argument, *ir, *env);
      if (!result)
        return result;

      auto formal_argument_type = *cursor;
      if (!formal_argument_type->equals(result.value()))
        return {Error::Kind::ArgumentTypeMismatch};

      ++cursor;
    }

    return function_type->result_type();
  }

  Result<type::Ptr> operator()(ir::Unop &unop) noexcept {
    auto overloads = env->lookupUnop(unop.op());
    if (!overloads)
      return {Error::Kind::UnknownUnop};

    auto right = typecheck(unop.right(), *ir, *env);
    if (!right)
      return right;

    auto instance = overloads->lookup(right.value());
    if (!instance)
      return {Error::Kind::UnopTypeMismatch};

    return instance->result_type;
  }

  Result<type::Ptr> operator()([[maybe_unused]] ir::Import &import) noexcept {
    // #TODO: handle import
    return env->getNilType();
  }

  Result<type::Ptr> operator()(ir::Module &module_) noexcept {
    env->pushScope(module_.name());

    for (auto &expression : module_.expressions()) {
      auto result = typecheck(expression, *env);
      if (!result) {
        env->unbindScope(module_.name());
        env->popScope();
        return result;
      }
    }

    env->popScope();
    return env->getNilType();
  }

  Result<type::Ptr> operator()(ir::Lambda &lambda) noexcept {
    env->pushScope();

    auto &formal_arguments = lambda.arguments();

    type::Function::Arguments argument_types;
    argument_types.reserve(formal_arguments.size());

    for (auto &formal_argument : formal_arguments) {
      argument_types.emplace_back(formal_argument.type);
      env->declareName(formal_argument.name, formal_argument.attributes,
                       formal_argument.type);
    }

    auto result = typecheck(lambda.body(), *ir, *env);
    if (!result) {
      env->popScope();
      return result;
    }

    // #TODO: handle optional lambda result_type

    auto function_type =
        env->getFunctionType(result.value(), std::move(argument_types));
    auto lambda_type = env->getLambdaType(function_type);

    env->popScope();
    return lambda_type;
  }
};

Result<type::Ptr> typecheck(ir::detail::Index &index, ir::Mir &ir,
                            Environment &env) noexcept {
  TypecheckInstruction visitor(ir, env);
  return visitor(index);
}

Result<type::Ptr> typecheck(ir::Mir &ir, Environment &env) noexcept {
  ir::detail::Index index{0};
  return typecheck(index, ir, env);
}
} // namespace mint
