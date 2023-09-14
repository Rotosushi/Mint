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

namespace mint {
struct EvaluateAst {
  ast::Ptr &ptr;
  Environment &env;

  EvaluateAst(ast::Ptr &ptr, Environment &env) noexcept : ptr(ptr), env(env) {}

  Result<ast::Ptr> operator()() noexcept {
    return std::visit(*this, ptr->variant);
  }

  Result<ast::Ptr> operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return ptr;
  }

  Result<ast::Ptr> operator()([[maybe_unused]] bool &b) noexcept { return ptr; }

  Result<ast::Ptr> operator()([[maybe_unused]] int &i) noexcept { return ptr; }

  Result<ast::Ptr> operator()(Identifier &i) noexcept {
    auto result = env.lookupBinding(i);
    if (!result) {
      return Error{Error::Kind::NameUnboundInScope, ptr->sl, i.view()};
    }

    auto binding = result.value();
    if (!binding.hasComptimeValue()) {
      // #NOTE: if the binding does not yet have a comptime value
      // that means it's definition hasn't been evaluated yet.
      // which means the definition is sitting in the ubd map.
      // which means wherever this current variable is sitting
      // needs to be delayed. this is communicated via Recovered.
      return Recovered{};
    }

    return binding.comptimeValueOrAssert();
  }

  Result<ast::Ptr> operator()([[maybe_unused]] ast::Lambda &l) noexcept {
    return ptr;
  }

  Result<ast::Ptr> operator()(ast::Function &f) noexcept {
    auto found = env.lookupLocalBinding(f.name);
    if (!found) {
      return Error{Error::Kind::NameUnboundInScope, ptr->sl, f.name};
    }
    auto binding = found.value();

    if (binding.hasComptimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl, f.name};
    }

    binding.setComptimeValue(ptr);

    if (auto failed =
            env.resolveComptimeValueOfUseBeforeDef(env.qualifyName(f.name))) {
      return failed.value();
    }

    return ast::create();
  }

  Result<ast::Ptr> operator()(ast::Let &l) noexcept {
    auto found = env.lookupLocalBinding(l.name);
    if (!found) {
      return Error{Error::Kind::NameUnboundInScope, ptr->sl, l.name};
    }
    auto binding = found.value();

    if (binding.hasComptimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl, l.name};
    }

    auto result = evaluate(l.affix, env);
    if (!result) {
      return result;
    }

    binding.setComptimeValue(result.value());

    if (auto failed =
            env.resolveComptimeValueOfUseBeforeDef(env.qualifyName(l.name))) {
      return failed.value();
    }

    return ast::create();
  }

  Result<ast::Ptr> operator()(ast::Binop &b) noexcept {
    auto overloads = env.lookupBinop(b.op);
    if (!overloads) {
      return Error{Error::Kind::UnknownBinop, ptr->sl, tokenToView(b.op)};
    }

    auto left_result = evaluate(b.left, env);
    if (!left_result) {
      return left_result;
    }
    auto &left = left_result.value();
    auto left_type = b.left->cached_type;
    MINT_ASSERT(left_type != nullptr);

    auto right_result = evaluate(b.right, env);
    if (!right_result) {
      return right_result;
    }
    auto &right = right_result.value();
    auto right_type = b.right->cached_type;
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(left_type, right_type);
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Types [" << left_type << ", " << right_type << "]";
      return Error{Error::Kind::BinopTypeMismatch, ptr->sl, msg.view()};
    }

    return instance->evaluate(left, right);
  }

  Result<ast::Ptr> operator()(ast::Unop &u) noexcept {
    auto overloads = env.lookupUnop(u.op);
    if (!overloads) {
      return Error{Error::Kind::UnknownUnop, ptr->sl, tokenToView(u.op)};
    }

    auto right_result = evaluate(u.right, env);
    if (!right_result) {
      return right_result;
    }
    auto &right = right_result.value();
    auto right_type = u.right->cached_type;
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(right_type);
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Type [" << right_type << "]";
      return Error{Error::Kind::UnopTypeMismatch, ptr->sl, msg.view()};
    }

    return instance->evaluate(right);
  }

  Result<ast::Ptr> operator()(ast::Call &c) noexcept {
    auto callee_type = c.callee->cached_type;
    MINT_ASSERT(callee_type != nullptr);
    MINT_ASSERT(type::callable(callee_type));

    auto callee_result = evaluate(c.callee, env);
    if (!callee_result) {
      return callee_result;
    }
    auto &callee = callee_result.value();

    auto [args, body] = [&]() {
      if (callee->holds<ast::Lambda>()) {
        auto &lambda = callee->get<ast::Lambda>();
        return std::make_pair(std::ref(lambda.arguments),
                              std::ref(lambda.body));
      } else if (callee->holds<ast::Function>()) {
        auto &function = callee->get<ast::Function>();
        return std::make_pair(std::ref(function.arguments),
                              std::ref(function.body));
      } else {
        abort("bad callee type.");
      }
    }();

    env.pushScope();
    for (auto &arg : args) {
      env.declareName(arg);
    }

    // #NOTE: the return value of a function is the
    // last expression in it's body
    ast::Ptr return_value;
    for (auto &expression : body) {
      auto result = evaluate(expression, env);
      if (!result) {
        env.popScope();
        return result;
      }

      return_value = result.value();
    }
    env.popScope();

    return return_value;
  }

  Result<ast::Ptr> operator()(ast::Parens &p) noexcept {
    return evaluate(p.expression, env);
  }

  Result<ast::Ptr> operator()(ast::Import &i) noexcept {
    auto *itu = env.findImport(i.file);
    MINT_ASSERT(itu != nullptr);

    auto &context = itu->context();
    if (context.evaluated()) {
      return ast::create();
    }

    for (auto &expression : itu->expressions()) {
      auto result = evaluate(expression, env);
      if (result.recovered()) {
        continue;
      } else if (!result) {
        env.errorStream() << result.error() << "\n";
        return Error{Error::Kind::ImportFailed, ptr->sl, i.file};
      }
    }

    context.evaluated(true);
    return ast::create();
  }

  Result<ast::Ptr> operator()(ast::Module &m) noexcept {
    env.pushScope(m.name);
    for (auto &expression : m.expressions) {
      auto result = evaluate(expression, env);
      if (!result) {
        env.unbindScope(m.name);
        env.popScope();
        return result;
      }
    }
    env.popScope();
    return ast::create();
  }
};

Result<ast::Ptr> evaluate(ast::Ptr &ptr, Environment &env) noexcept {
  MINT_ASSERT(ptr->cached_type != nullptr);
  EvaluateAst visitor(ptr, env);
  return visitor();
}

int evaluate(Environment &env) noexcept {
  for (auto &expression : env.localExpressions()) {
    auto result = evaluate(expression, env);
    if (!result) {
      env.errorStream() << result.error() << "\n";
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
} // namespace mint
// struct EvalauteImmediate {
//   Environment *env;

//   EvalauteImmediate(Environment &env) noexcept : env(&env) {}

//   Result<ir::Value> operator()(ir::detail::Immediate &immediate) noexcept {
//     return std::visit(*this, immediate.variant());
//   }

//   Result<ir::Value> operator()(ir::Scalar &scalar) noexcept { return
//   {scalar}; }

//   Result<ir::Value> operator()(Identifier &name) noexcept {
//     auto result = env->lookupBinding(name);
//     MINT_ASSERT(result.success());
//     auto binding = result.value();

//     if (!binding.hasComptimeValue()) {
//       return Recovered{};
//     }

//     return binding.comptimeValueOrAssert();
//   }
// };

// static Result<ir::Value> evaluate(ir::detail::Immediate &immediate,
//                                   Environment &env) noexcept {
//   EvalauteImmediate visitor(env);
//   return visitor(immediate);
// }

// static Result<ir::Value> evaluate(ir::detail::Index index, ir::Mir &mir,
//                                   Environment &env) noexcept;

// static Result<ir::Value> evaluate(ir::detail::Parameter &parameter,
//                                   ir::Mir &mir, Environment &env) noexcept {
//   return evaluate(parameter.index(), mir, env);
// }

// struct EvaluateInstruction {
//   ir::Mir *mir;
//   ir::detail::Index index;
//   Environment *env;

//   EvaluateInstruction(ir::Mir &mir, ir::detail::Index index,
//                       Environment &env) noexcept
//       : mir(&mir), index(index), env(&env) {}

//   Result<ir::Value> operator()() noexcept {
//     return std::visit(*this, (*mir)[index].variant());
//   }

//   Result<ir::Value> operator()(ir::detail::Immediate &immediate) noexcept {
//     MINT_ASSERT(immediate.cachedType() != nullptr);
//     return evaluate(immediate, *env);
//   }

//   Result<ir::Value> operator()(ir::Parens &parens) noexcept {
//     MINT_ASSERT(parens.cachedType() != nullptr);
//     return evaluate(parens.parameter(), *mir, *env);
//   }

//   Result<ir::Value> operator()(ir::Let &let) noexcept {
//     MINT_ASSERT(let.cachedType() != nullptr);
//     auto found = env->lookupLocalBinding(let.name());
//     MINT_ASSERT(found);
//     auto binding = found.value();

//     if (binding.hasComptimeValue()) {
//       return Error{Error::Kind::NameAlreadyBoundInScope,
//       let.sourceLocation(),
//                    let.name()};
//     }

//     auto result = evaluate(let.parameter(), *mir, *env);
//     if (!result) {
//       return result;
//     }

//     binding.setComptimeValue(result.value());

//     auto qualifiedName = env->qualifyName(let.name());
//     if (auto failed = env->resolveComptimeValueOfUseBeforeDef(qualifiedName))
//     {
//       return failed.value();
//     }

//     return ir::Value{};
//   }

//   Result<ir::Value> operator()(ir::Function &function) noexcept {
//     // what do we bind in the symbol table such that call expressions
//     // can apply this function? I think the choice is a lambda.
//     // for a few reasons:
//     // -) lambdas are already values
//     // -) lambdas are callables
//     // -) we can construct a lambda out of any function by cloning
//     // -) when we codegen or forward declare, we don't use the definition
//     //    within the symbol table, so we don't codegen/forward declare
//     //    a lambda, we process the function.
//     // -) we already store lambdas within the symbl table.
//     // -) we want to coerce functions into lambda's to treat them
//     //    as values.
//     //
//     // as a whole this has made me reconsider evaluating with the
//     // tree structure, simply due to avoiding copying large functions.
//   }

//   Result<ir::Value> operator()(ir::Binop &binop) noexcept {
//     MINT_ASSERT(binop.cachedType() != nullptr);
//     auto overloads = env->lookupBinop(binop.op());
//     MINT_ASSERT(overloads);

//     auto left_result = evaluate(binop.left(), *mir, *env);
//     if (!left_result) {
//       return left_result;
//     }
//     auto &left_value = left_result.value();
//     MINT_ASSERT(left_value.holds<ir::Scalar>());
//     auto &left = left_value.get<ir::Scalar>();
//     auto left_type = binop.left().cachedType();
//     MINT_ASSERT(left_type != nullptr);

//     auto right_result = evaluate(binop.right(), *mir, *env);
//     if (!right_result) {
//       return right_result;
//     }
//     auto &right_value = right_result.value();
//     MINT_ASSERT(right_value.holds<ir::Scalar>());
//     auto &right = right_value.get<ir::Scalar>();
//     auto right_type = binop.right().cachedType();
//     MINT_ASSERT(right_type != nullptr);

//     auto instance = overloads->lookup(left_type, right_type);
//     MINT_ASSERT(instance);

//     return instance->evaluate(left, right);
//   }

//   Result<ir::Value> operator()(ir::Unop &unop) noexcept {
//     MINT_ASSERT(unop.cachedType() != nullptr);
//     auto overloads = env->lookupUnop(unop.op());
//     MINT_ASSERT(overloads);

//     auto right_type = unop.right().cachedType();
//     MINT_ASSERT(right_type != nullptr);
//     auto right_result = evaluate(unop.right(), *mir, *env);
//     if (!right_result) {
//       return right_result;
//     }
//     auto &right_value = right_result.value();
//     MINT_ASSERT(right_value.holds<ir::Scalar>());
//     auto &right = right_value.get<ir::Scalar>();

//     auto instance = overloads->lookup(right_type);
//     MINT_ASSERT(instance);

//     return instance->evaluate(right);
//   }

//   // Result<ir::Value> operator()(ir::Call &call) noexcept {
//   //   auto callee_type = call.callee().cachedType();
//   //   MINT_ASSERT(callee_type != nullptr);
//   //   MINT_ASSERT(type::callable(callee_type));

//   //   auto callee_result = evaluate(call.callee(), *mir, *env);
//   //   if (!callee_result) {
//   //     return callee_result;
//   //   }
//   //   MINT_ASSERT(callee_result.value().holds<ir::Lambda>());
//   //   auto &callee = callee_result.value().get<ir::Lambda>();

//   //   std::vector<ir::Value> actual_arguments;
//   //   actual_arguments.reserve(call.arguments().size());
//   //   for (auto &argument : call.arguments()) {
//   //     auto result = evaluate(argument, *mir, *env);
//   //     if (!result) {
//   //       return result;
//   //     }

//   //     actual_arguments.emplace_back(std::move(result.value()));
//   //   }

//   //   env->pushScope();
//   //   auto formal_cursor = callee.arguments().begin();
//   //   for (auto &argument : actual_arguments) {
//   //     auto &formal_argument = *formal_cursor;
//   //     auto bound = env->declareName(formal_argument);
//   //     if (!bound) {
//   //       env->popScope();
//   //       return bound.error();
//   //     }
//   //     auto binding = bound.value();

//   //     binding.setComptimeValue(argument);

//   //     ++formal_cursor;
//   //   }

//   //   auto result = evaluate(callee.body(), *env);

//   //   env->popScope();
//   //   return result;
//   // }

//   // Result<ir::Value> operator()(ir::Lambda &lambda) noexcept {
//   //   return ir::Value{lambda};
//   // }

//   Result<ir::Value> operator()(ir::Import &i) noexcept {
//     MINT_ASSERT(i.cachedType() != nullptr);
//     auto *itu = env->findImport(i.file());
//     MINT_ASSERT(itu != nullptr);

//     auto &context = itu->context();
//     if (context.evaluated()) {
//       return ir::Value{};
//     }

//     std::size_t index = 0U;
//     auto &recovered_expressions = itu->recovered_expressions();
//     for (auto &expression : itu->expressions()) {
//       if (!recovered_expressions[index]) {
//         auto result = evaluate(expression, *env);
//         if (result.recovered()) {
//           recovered_expressions[index] = true;
//         } else if (!result) {
//           env->errorStream() << result.error() << "\n";
//           return Error{Error::Kind::ImportFailed, i.sourceLocation(),
//           i.file()};
//         }
//       }
//       ++index;
//     }

//     context.evaluated(true);
//     return ir::Value{};
//   }

//   Result<ir::Value> operator()(ir::Module &m) noexcept {
//     MINT_ASSERT(m.cachedType() != nullptr);
//     env->pushScope(m.name());

//     std::size_t index = 0U;
//     auto &recovered_expressions = m.recovered_expressions();
//     for (auto &expression : m.expressions()) {
//       // #NOTE:
//       // if the current expression was recovered during
//       // typechecking we skip evaluating it here.
//       if (!recovered_expressions[index]) {
//         auto result = evaluate(expression, *env);
//         if (result.recovered()) {
//           recovered_expressions[index] = true;
//         } else if (!result) {
//           env->unbindScope(m.name());
//           env->popScope();
//           return result;
//         }
//       }
//       ++index;
//     }

//     env->popScope();
//     return ir::Value{};
//   }
// };

// static Result<ir::Value> evaluate(ir::detail::Index index, ir::Mir &mir,
//                                   Environment &env) noexcept {
//   EvaluateInstruction visitor(mir, index, env);
//   return visitor();
// }

// Result<ir::Value> evaluate(ir::Mir &mir, Environment &env) {
//   return evaluate(mir.root(), mir, env);
// }

// int evaluate(Environment &env) noexcept {
//   std::size_t index = 0U;
//   auto &recovered_expressions = env.localRecoveredExpressions();
//   for (auto &expression : env.localExpressions()) {
//     if (!recovered_expressions[index]) {
//       auto result = evaluate(expression, env);
//       if (result.recovered()) {
//         recovered_expressions[index] = true;
//       } else if (!result) {
//         env.errorStream() << result.error() << "\n";
//         return EXIT_FAILURE;
//       }
//     }
//     ++index;
//   }

//   return EXIT_SUCCESS;
// }
