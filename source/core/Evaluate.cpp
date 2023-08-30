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
#include "core/Evaluate.hpp"
#include "adt/Environment.hpp"
#include "core/Repl.hpp"
#include "core/Typecheck.hpp"
#include "ir/Instruction.hpp"

namespace mint {
struct EvalauteImmediate {
  Environment *env;

  EvalauteImmediate(Environment &env) noexcept : env(&env) {}

  Result<ir::Value> operator()(ir::detail::Immediate &immediate) noexcept {
    return std::visit(*this, immediate.variant());
  }

  Result<ir::Value> operator()(ir::Scalar &scalar) noexcept { return {scalar}; }

  Result<ir::Value> operator()(Identifier &name) noexcept {
    auto result = env->lookupBinding(name);
    MINT_ASSERT(result.success());
    auto binding = result.value();

    // #NOTE: can we just return "Recovered" here?
    // we are planning on only inserting mir
    // into the UBDMap during typechecking,
    // and the only reason we have for returning
    // this information is to bind mir into the
    // UBDMap...
    if (!binding.hasComptimeValue()) {
      return Recovered{};
    }

    return binding.comptimeValue().value();
  }
};

static Result<ir::Value> evaluate(ir::detail::Immediate &immediate,
                                  Environment &env) {
  EvalauteImmediate visitor(env);
  return visitor(immediate);
}

static Result<ir::Value> evaluate(ir::detail::Index index, ir::Mir &mir,
                                  Environment &env) noexcept;

static Result<ir::Value> evaluate(ir::detail::Parameter &parameter,
                                  ir::Mir &mir, Environment &env) noexcept {
  return evaluate(parameter.index(), mir, env);
}

struct EvaluateInstruction {
  ir::Mir *mir;
  ir::detail::Index index;
  Environment *env;

  EvaluateInstruction(ir::Mir &mir, ir::detail::Index index,
                      Environment &env) noexcept
      : mir(&mir), index(index), env(&env) {}

  Result<ir::Value> operator()() noexcept {
    return std::visit(*this, (*mir)[index].variant());
  }

  Result<ir::Value> operator()(ir::detail::Immediate &immediate) noexcept {
    return evaluate(immediate, *env);
  }

  Result<ir::Value> operator()(ir::Parens &parens) noexcept {
    return evaluate(parens.parameter(), *mir, *env);
  }

  Result<ir::Value> operator()(ir::Let &let) noexcept {
    auto found = env->lookupLocalBinding(let.name());
    MINT_ASSERT(found);
    auto binding = found.value();

    if (binding.hasComptimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, let.sourceLocation(),
                   let.name()};
    }

    auto result = evaluate(let.parameter(), *mir, *env);
    if (!result) {
      return result;
    }

    binding.setComptimeValue(result.value());

    auto qualifiedName = env->qualifyName(let.name());
    if (auto failed = env->resolveComptimeValueOfUseBeforeDef(qualifiedName)) {
      return failed.value();
    }

    return ir::Value{};
  }

  Result<ir::Value> operator()(ir::Binop &binop) noexcept {
    auto overloads = env->lookupBinop(binop.op());
    MINT_ASSERT(overloads);

    auto left = evaluate(binop.left(), *mir, *env);
    if (!left) {
      return left;
    }
    auto &left_value = left.value();
    MINT_ASSERT(left_value.holds<ir::Scalar>());
    auto left_type = binop.left().cachedType();
    MINT_ASSERT(left_type != nullptr);

    auto right = evaluate(binop.right(), *mir, *env);
    if (!right) {
      return right;
    }
    auto right_value = right.value();
    MINT_ASSERT(right_value.holds<ir::Scalar>());
    auto right_type = binop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(left_type, right_type);
    MINT_ASSERT(instance);

    return instance->evaluate(left_value.get<ir::Scalar>(),
                              right_value.get<ir::Scalar>());
  }

  Result<ir::Value> operator()(ir::Unop &unop) noexcept {
    auto overloads = env->lookupUnop(unop.op());
    MINT_ASSERT(overloads);

    auto right_type = unop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);
    auto right_result = evaluate(unop.right(), *mir, *env);
    if (!right_result) {
      return right_result;
    }
    auto right = right_result.value();
    MINT_ASSERT(right.holds<ir::Scalar>());

    auto instance = overloads->lookup(right_type);
    MINT_ASSERT(instance);

    return instance->evaluate(right.get<ir::Scalar>());
  }

  Result<ir::Value> operator()(ir::Call &call) noexcept {
    auto callee_type = call.callee().cachedType();
    MINT_ASSERT(callee_type != nullptr);
    MINT_ASSERT(type::callable(callee_type));

    auto callee_result = evaluate(call.callee(), *mir, *env);
    if (!callee_result) {
      return callee_result;
    }
    MINT_ASSERT(callee_result.value().holds<ir::Lambda>());
    auto &callee = callee_result.value().get<ir::Lambda>();

    std::vector<ir::Value> actual_arguments;
    actual_arguments.reserve(call.arguments().size());
    for (auto &argument : call.arguments()) {
      auto result = evaluate(argument, *mir, *env);
      if (!result) {
        return result;
      }

      actual_arguments.emplace_back(std::move(result.value()));
    }

    env->pushScope();
    auto formal_cursor = callee.arguments().begin();
    for (auto &argument : actual_arguments) {
      auto &formal_argument = *formal_cursor;
      auto bound =
          env->declareName(formal_argument.name, formal_argument.attributes,
                           formal_argument.type);
      if (!bound) {
        env->popScope();
        return bound.error();
      }
      auto binding = bound.value();

      binding.setComptimeValue(argument);

      ++formal_cursor;
    }

    auto result = evaluate(callee.body(), *env);

    env->popScope();
    return result;
  }

  Result<ir::Value> operator()(ir::Import &import) noexcept {
    if (env->alreadyImported(import.file())) {
      return ir::Value{};
    }

    auto found = env->fileSearch(import.file());
    MINT_ASSERT(found);
    env->pushActiveSourceFile(std::move(found.value()));

    if (repl(*env, false) == EXIT_FAILURE) {
      return Error{Error::Kind::ImportFailed, import.sourceLocation(),
                   import.file()};
    }

    env->popActiveSourceFile();
    env->addImport(import.file());
    return ir::Value{};
  }

  Result<ir::Value> operator()(ir::Module &m) noexcept {
    env->pushScope(m.name());

    std::size_t index = 0U;
    auto &recovered_expressions = m.recovered_expressions();
    for (auto &expression : m.expressions()) {
      // #NOTE:
      // if the current expression was recovered during
      // typechecking we skip evaluating it here.
      if (!recovered_expressions[index]) {
        auto result = evaluate(expression, *env);
        if (result.recovered()) {
          continue;
        }

        if (!result) {
          env->unbindScope(m.name());
          env->popScope();
          return result;
        }
      }
      ++index;
    }

    env->popScope();
    return ir::Value{};
  }

  Result<ir::Value> operator()(ir::Lambda &lambda) noexcept {
    return ir::Value{lambda};
  }
};

static Result<ir::Value> evaluate(ir::detail::Index index, ir::Mir &mir,
                                  Environment &env) noexcept {
  EvaluateInstruction visitor(mir, index, env);
  return visitor();
}

Result<ir::Value> evaluate(ir::Mir &mir, Environment &env) {
  return evaluate(mir.root(), mir, env);
}
} // namespace mint
