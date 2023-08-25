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
#include "core/Typecheck.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"
#include "ir/action/Clone.hpp"
#include "utility/Abort.hpp"

namespace mint {
struct TypecheckScalar {
  Environment *env;

  TypecheckScalar(Environment &env) noexcept : env(&env) {}

  Result<type::Ptr> operator()(ir::Scalar &scalar) noexcept {
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

static Result<type::Ptr> typecheck(ir::Scalar &scalar,
                                   Environment &env) noexcept {
  TypecheckScalar visitor(env);
  return visitor(scalar);
}

struct TypecheckImmediate {
  Environment *env;

  TypecheckImmediate(Environment &env) noexcept : env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Immediate &immediate) noexcept {
    return std::visit(*this, immediate.variant());
  }

  Result<type::Ptr> operator()(ir::Scalar &scalar) noexcept {
    return typecheck(scalar, *env);
  }

  Result<type::Ptr> operator()(Identifier &name) noexcept {
    auto result = env->lookupBinding(name);
    if (!result) {
      auto error = result.error();
      if (error.kind() != Error::Kind::NameUnboundInScope)
        return {error.kind()};

      return Recoverable{name, env->nearestNamedScope()};
    }

    auto type = result.value().type();
    // #TODO: create a type cache
    return type;
  }
};

static Result<type::Ptr> typecheck(ir::detail::Immediate &immediate,
                                   Environment &env) noexcept {
  TypecheckImmediate visitor(env);
  return visitor(immediate);
}

static Result<type::Ptr> typecheck(ir::detail::Index index, ir::Mir &ir,
                                   Environment &env) noexcept;

struct TypecheckParameter {
  ir::Mir *ir;
  Environment *env;

  TypecheckParameter(ir::Mir &ir, Environment &env) noexcept
      : ir(&ir), env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Parameter &parameter) noexcept {
    auto cached_type = parameter.cachedType();
    if (cached_type != nullptr) {
      return cached_type;
    }

    auto result = std::visit(*this, parameter.variant());
    if (!result) {
      return result;
    }

    parameter.cachedType(result.value());
    return result;
  }

  Result<type::Ptr> operator()(ir::detail::Immediate &immediate) noexcept {
    return typecheck(immediate, *env);
  }

  Result<type::Ptr> operator()(ir::detail::Index &index) noexcept {
    return typecheck(index, *ir, *env);
  }
};

static Result<type::Ptr> typecheck(ir::detail::Parameter &parameter,
                                   ir::Mir &ir, Environment &env) noexcept {
  TypecheckParameter visitor(ir, env);
  return visitor(parameter);
}

struct RecoverableErrorVisitor {
  Recoverable *recoverable;
  ir::Mir *ir;
  ir::detail::Index index;
  Identifier m_def;
  Environment *m_env;

  RecoverableErrorVisitor(Recoverable &recoverable, ir::Mir &ir,
                          ir::detail::Index index, Identifier def,
                          Environment &env) noexcept
      : recoverable(&recoverable), ir(&ir), index(index), m_def(def),
        m_env(&env) {}

  Result<type::Ptr> operator()() noexcept {
    return std::visit(*this, recoverable->data());
  }

  Result<type::Ptr>
  operator()([[maybe_unused]] std::monostate const &nil) noexcept {
    return {Error::Kind::Default};
  }

  Result<type::Ptr> operator()(Recoverable::UBD const &ubd) noexcept {
    if (auto failed = m_env->bindUseBeforeDef(
            ubd.undef_name, m_def, ubd.local_scope, clone(*ir, index))) {
      return failed.value();
    }

    return Recovered{};
  }
};

// #TODO: this function has way too many arguments.
// but all of them are necessary. So I guess ill let it slide.
static Result<type::Ptr> handleRecoverableError(Recoverable &recoverable,
                                                ir::Mir &ir,
                                                ir::detail::Index index,
                                                Identifier def,
                                                Environment &env) noexcept {
  RecoverableErrorVisitor visitor(recoverable, ir, index, def, env);
  return visitor();
}

struct TypecheckInstruction {
  ir::Mir *ir;
  ir::detail::Index index;
  Environment *env;

  TypecheckInstruction(ir::Mir &ir, ir::detail::Index index,
                       Environment &env) noexcept
      : ir(&ir), index(index), env(&env) {}

  Result<type::Ptr> operator()() noexcept {
    auto &instruction = (*ir)[index];

    auto cached_type = instruction.cachedType();
    if (cached_type != nullptr) {
      return cached_type;
    }

    auto result = std::visit(*this, instruction.variant());
    if (!result) {
      return result;
    }

    instruction.cachedType(result.value());
    return result;
  }

  Result<type::Ptr> operator()(ir::Affix &affix) noexcept {
    return typecheck(affix.parameter(), *ir, *env);
  }

  Result<type::Ptr> operator()(ir::Parens &parens) noexcept {
    return typecheck(parens.parameter(), *ir, *env);
  }

  Result<type::Ptr> operator()(ir::Let &let) noexcept {
    auto found = env->lookupLocalBinding(let.name());
    if (found) // #TODO: better error messages
      return {Error::Kind::NameAlreadyBoundInScope};

    auto result = typecheck(let.parameter(), *ir, *env);
    if (!result) {
      if (result.recoverable()) {
        return handleRecoverableError(result.unknown(), *ir, index,
                                      env->qualifyName(let.name()), *env);
      }

      return result;
    }
    auto type = result.value();

    // #TODO: handle optional type annotation
    // #TODO: add bound variable attributes
    if (auto bound = env->declareName(let.name(), {}, type); !bound) {
      return bound.error();
    }

    auto qualifiedName = env->qualifyName(let.name());
    if (auto failed = env->resolveTypeOfUseBeforeDef(qualifiedName))
      return failed.value();

    return env->getNilType();
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

    auto callee_type = callee.value();
    if (!type::callable(callee_type))
      return {Error::Kind::CannotCallType};

    type::Function *function_type = nullptr;
    auto &variant = callee_type->variant;
    if (std::holds_alternative<type::Lambda>(variant)) {
      auto lambda_type = std::get_if<type::Lambda>(&variant);
      auto ptr = lambda_type->function_type;
      function_type = std::get_if<type::Function>(&ptr->variant);

    } else if (std::holds_alternative<type::Function>(variant)) {
      function_type = std::get_if<type::Function>(&variant);

    } else {
      abort("bad callable type!");
    }
    MINT_ASSERT(function_type != nullptr);

    auto &actual_arguments = call.arguments();
    auto &formal_arguments = function_type->arguments;
    if (formal_arguments.size() != actual_arguments.size())
      return {Error::Kind::ArgumentNumberMismatch};

    auto cursor = formal_arguments.begin();
    for (auto &actual_argument : actual_arguments) {
      auto result = typecheck(actual_argument, *ir, *env);
      if (!result)
        return result;

      auto formal_argument_type = *cursor;
      if (!type::equals(formal_argument_type, result.value()))
        return {Error::Kind::ArgumentTypeMismatch};

      ++cursor;
    }

    return function_type->result_type;
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

  Result<type::Ptr> operator()(ir::Import &import) noexcept {
    if (env->alreadyImported(import.file()))
      return env->getNilType();

    if (!env->fileExists(import.file()))
      return {Error::Kind::FileNotFound};

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

    auto result = typecheck(lambda.body(), *env);
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

static Result<type::Ptr> typecheck(ir::detail::Index index, ir::Mir &ir,
                                   Environment &env) noexcept {
  TypecheckInstruction visitor(ir, index, env);
  return visitor();
}

Result<type::Ptr> typecheck(ir::Mir &ir, Environment &env) noexcept {
  return typecheck(ir.root(), ir, env);
}
} // namespace mint
