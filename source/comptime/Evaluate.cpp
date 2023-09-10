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
#include "comptime/Evaluate.hpp"
#include "adt/Environment.hpp"
#include "comptime/Import.hpp"
#include "comptime/Typecheck.hpp"
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

    if (!binding.hasComptimeValue()) {
      return Recovered{};
    }

    return binding.comptimeValueOrAssert();
  }
};

static Result<ir::Value> evaluate(ir::detail::Immediate &immediate,
                                  Environment &env) noexcept {
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
    MINT_ASSERT(immediate.cachedType() != nullptr);
    return evaluate(immediate, *env);
  }

  Result<ir::Value> operator()(ir::Parens &parens) noexcept {
    MINT_ASSERT(parens.cachedType() != nullptr);
    return evaluate(parens.parameter(), *mir, *env);
  }

  Result<ir::Value> operator()(ir::Let &let) noexcept {
    MINT_ASSERT(let.cachedType() != nullptr);
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

  Result<ir::Value> operator()(ir::Function &function) noexcept {
    // what do we bind in the symbol table such that call expressions
    // can apply this function? I think the choice is a lambda.
    // for a few reasons:
    // -) lambdas are already values
    // -) lambdas are callables
    // -) we can construct a lambda out of any function by cloning
    // -) when we codegen or forward declare, we don't use the definition
    //    within the symbol table, so we don't codegen/forward declare
    //    a lambda, we process the function.
    // -) we already store lambdas within the symbl table.
    // -) we want to coerce functions into lambda's to treat them
    //    as values.
    
  }

  Result<ir::Value> operator()(ir::Binop &binop) noexcept {
    MINT_ASSERT(binop.cachedType() != nullptr);
    auto overloads = env->lookupBinop(binop.op());
    MINT_ASSERT(overloads);

    auto left_result = evaluate(binop.left(), *mir, *env);
    if (!left_result) {
      return left_result;
    }
    auto &left_value = left_result.value();
    MINT_ASSERT(left_value.holds<ir::Scalar>());
    auto &left = left_value.get<ir::Scalar>();
    auto left_type = binop.left().cachedType();
    MINT_ASSERT(left_type != nullptr);

    auto right_result = evaluate(binop.right(), *mir, *env);
    if (!right_result) {
      return right_result;
    }
    auto &right_value = right_result.value();
    MINT_ASSERT(right_value.holds<ir::Scalar>());
    auto &right = right_value.get<ir::Scalar>();
    auto right_type = binop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(left_type, right_type);
    MINT_ASSERT(instance);

    return instance->evaluate(left, right);
  }

  Result<ir::Value> operator()(ir::Unop &unop) noexcept {
    MINT_ASSERT(unop.cachedType() != nullptr);
    auto overloads = env->lookupUnop(unop.op());
    MINT_ASSERT(overloads);

    auto right_type = unop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);
    auto right_result = evaluate(unop.right(), *mir, *env);
    if (!right_result) {
      return right_result;
    }
    auto &right_value = right_result.value();
    MINT_ASSERT(right_value.holds<ir::Scalar>());
    auto &right = right_value.get<ir::Scalar>();

    auto instance = overloads->lookup(right_type);
    MINT_ASSERT(instance);

    return instance->evaluate(right);
  }

  // Result<ir::Value> operator()(ir::Call &call) noexcept {
  //   auto callee_type = call.callee().cachedType();
  //   MINT_ASSERT(callee_type != nullptr);
  //   MINT_ASSERT(type::callable(callee_type));

  //   auto callee_result = evaluate(call.callee(), *mir, *env);
  //   if (!callee_result) {
  //     return callee_result;
  //   }
  //   MINT_ASSERT(callee_result.value().holds<ir::Lambda>());
  //   auto &callee = callee_result.value().get<ir::Lambda>();

  //   std::vector<ir::Value> actual_arguments;
  //   actual_arguments.reserve(call.arguments().size());
  //   for (auto &argument : call.arguments()) {
  //     auto result = evaluate(argument, *mir, *env);
  //     if (!result) {
  //       return result;
  //     }

  //     actual_arguments.emplace_back(std::move(result.value()));
  //   }

  //   env->pushScope();
  //   auto formal_cursor = callee.arguments().begin();
  //   for (auto &argument : actual_arguments) {
  //     auto &formal_argument = *formal_cursor;
  //     auto bound = env->declareName(formal_argument);
  //     if (!bound) {
  //       env->popScope();
  //       return bound.error();
  //     }
  //     auto binding = bound.value();

  //     binding.setComptimeValue(argument);

  //     ++formal_cursor;
  //   }

  //   auto result = evaluate(callee.body(), *env);

  //   env->popScope();
  //   return result;
  // }

  // Result<ir::Value> operator()(ir::Lambda &lambda) noexcept {
  //   return ir::Value{lambda};
  // }

  Result<ir::Value> operator()(ir::Import &i) noexcept {
    MINT_ASSERT(i.cachedType() != nullptr);
    auto *itu = env->findImport(i.file());
    MINT_ASSERT(itu != nullptr);

    auto &context = itu->context();
    if (context.evaluated()) {
      return ir::Value{};
    }

    std::size_t index = 0U;
    auto &recovered_expressions = itu->recovered_expressions();
    for (auto &expression : itu->expressions()) {
      if (!recovered_expressions[index]) {
        auto result = evaluate(expression, *env);
        if (result.recovered()) {
          recovered_expressions[index] = true;
        } else if (!result) {
          env->errorStream() << result.error() << "\n";
          return Error{Error::Kind::ImportFailed, i.sourceLocation(), i.file()};
        }
      }
      ++index;
    }

    context.evaluated(true);
    return ir::Value{};
  }

  Result<ir::Value> operator()(ir::Module &m) noexcept {
    MINT_ASSERT(m.cachedType() != nullptr);
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
          recovered_expressions[index] = true;
        } else if (!result) {
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
};

static Result<ir::Value> evaluate(ir::detail::Index index, ir::Mir &mir,
                                  Environment &env) noexcept {
  EvaluateInstruction visitor(mir, index, env);
  return visitor();
}

Result<ir::Value> evaluate(ir::Mir &mir, Environment &env) {
  return evaluate(mir.root(), mir, env);
}

int evaluate(Environment &env) noexcept {
  std::size_t index = 0U;
  auto &recovered_expressions = env.localRecoveredExpressions();
  for (auto &expression : env.localExpressions()) {
    if (!recovered_expressions[index]) {
      auto result = evaluate(expression, env);
      if (result.recovered()) {
        recovered_expressions[index] = true;
      } else if (!result) {
        env.errorStream() << result.error() << "\n";
        return EXIT_FAILURE;
      }
    }
    ++index;
  }

  return EXIT_SUCCESS;
}

} // namespace mint
