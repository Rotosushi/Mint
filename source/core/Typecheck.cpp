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
#include <sstream>

#include "adt/Environment.hpp"
#include "core/Typecheck.hpp"
#include "ir/Instruction.hpp"
#include "ir/actions/Clone.hpp"
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
    if (immediate.cachedType() != nullptr) {
      return immediate.cachedType();
    }

    auto result = std::visit(*this, immediate.variant());
    if (!result) {
      return result;
    }

    immediate.cachedType(result.value());
    return result;
  }

  Result<type::Ptr> operator()(ir::Scalar &scalar) noexcept {
    return typecheck(scalar, *env);
  }

  Result<type::Ptr> operator()(Identifier &name) noexcept {
    auto result = env->lookupBinding(name);
    if (!result) {
      auto error = result.error();
      if (error.kind() != Error::Kind::NameUnboundInScope)
        return Error{error.kind()};

      return Recoverable{name, env->nearestNamedScope()};
    }

    auto type = result.value().type();
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

static Result<type::Ptr> typecheck(ir::detail::Parameter &parameter,
                                   ir::Mir &ir, Environment &env) noexcept {
  if (parameter.cachedType() != nullptr) {
    return parameter.cachedType();
  }

  auto result = typecheck(parameter.index(), ir, env);
  if (!result) {
    return result;
  }

  parameter.cachedType(result.value());
  return result;
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
    return Error{Error::Kind::Default};
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
    return std::visit(*this, (*ir)[index].variant());
  }

  Result<type::Ptr> operator()(ir::detail::Immediate &immediate) noexcept {
    if (immediate.cachedType() != nullptr) {
      return immediate.cachedType();
    }

    return typecheck(immediate, *env);
  }

  Result<type::Ptr> operator()(ir::Parens &parens) noexcept {
    if (parens.cachedType() != nullptr) {
      return parens.cachedType();
    }

    auto result = typecheck(parens.parameter(), *ir, *env);
    if (!result) {
      return result;
    }

    parens.cachedType(result.value());
    return result;
  }

  Result<type::Ptr> operator()(ir::Let &let) noexcept {
    if (let.cachedType() != nullptr) {
      return let.cachedType();
    }

    auto found = env->lookupLocalBinding(let.name());
    if (found)
      return Error{Error::Kind::NameAlreadyBoundInScope, let.sourceLocation(),
                   let.name()};

    auto result = typecheck(let.parameter(), *ir, *env);
    if (!result) {
      if (result.recoverable()) {
        return handleRecoverableError(result.unknown(), *ir, index,
                                      env->qualifyName(let.name()), *env);
      }

      return result;
    }
    auto type = result.value();

    if (let.annotation()) {
      auto annotated_type = let.annotation().value();
      if (!equals(annotated_type, type)) {
        std::stringstream msg;
        msg << "Expected Type [" << annotated_type << "]"
            << " Actual Type [" << type << "]";
        return Error{Error::Kind::LetTypeMismatch, let.sourceLocation(),
                     msg.view()};
      }
    }

    if (auto bound = env->declareName(let.name(), let.attributes(), type);
        !bound) {
      return bound.error();
    }

    auto qualifiedName = env->qualifyName(let.name());
    if (auto failed = env->resolveTypeOfUseBeforeDef(qualifiedName))
      return failed.value();

    return let.cachedType(env->getNilType());
  }

  Result<type::Ptr> operator()(ir::Binop &binop) noexcept {
    if (binop.cachedType() != nullptr) {
      return binop.cachedType();
    }

    auto overloads = env->lookupBinop(binop.op());
    if (!overloads) {
      return Error{Error::Kind::UnknownBinop, binop.sourceLocation(),
                   tokenToView(binop.op())};
    }

    auto left = typecheck(binop.left(), *ir, *env);
    if (!left)
      return left;

    auto right = typecheck(binop.right(), *ir, *env);
    if (!right)
      return right;

    auto instance = overloads->lookup(left.value(), right.value());
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Types: [" << left.value() << ", " << right.value() << "]";
      return Error{Error::Kind::BinopTypeMismatch, binop.sourceLocation(),
                   msg.view()};
    }

    return binop.cachedType(instance->result_type);
  }

  Result<type::Ptr> operator()(ir::Unop &unop) noexcept {
    if (unop.cachedType() != nullptr) {
      return unop.cachedType();
    }

    auto overloads = env->lookupUnop(unop.op());
    if (!overloads) {
      return Error{Error::Kind::UnknownUnop, unop.sourceLocation(),
                   tokenToView(unop.op())};
    }

    auto right = typecheck(unop.right(), *ir, *env);
    if (!right)
      return right;

    auto instance = overloads->lookup(right.value());
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Type: [" << right.value() << "]";
      return Error{Error::Kind::UnopTypeMismatch, unop.sourceLocation(),
                   msg.view()};
    }

    return unop.cachedType(instance->result_type);
  }

  // Result<type::Ptr> operator()(ir::Call &call) noexcept {
  //   if (call.cachedType() != nullptr) {
  //     return call.cachedType();
  //   }

  //   auto callee = typecheck(call.callee(), *ir, *env);
  //   if (!callee)
  //     return callee;

  //   auto callee_type = callee.value();
  //   if (!type::callable(callee_type)) {
  //     std::stringstream msg;
  //     msg << "Callee Type: [" << callee_type << "]";
  //     return Error{Error::Kind::CannotCallType, call.sourceLocation(),
  //                  msg.view()};
  //   }

  //   type::Function *function_type = nullptr;
  //   if (callee_type->holds<type::Lambda>()) {
  //     auto &lambda_type = callee_type->get<type::Lambda>();
  //     auto ptr = lambda_type.function_type;
  //     function_type = &ptr->get<type::Function>();
  //   } else if (callee_type->holds<type::Function>()) {
  //     function_type = &callee_type->get<type::Function>();
  //   } else {
  //     abort("bad callable type!");
  //   }
  //   MINT_ASSERT(function_type != nullptr);

  //   auto &actual_arguments = call.arguments();
  //   auto &formal_arguments = function_type->arguments;
  //   if (formal_arguments.size() != actual_arguments.size()) {
  //     std::stringstream msg;
  //     msg << "Expected [" << formal_arguments.size() << "] arguments, ";
  //     msg << "Recieved [" << actual_arguments.size() << "] arguments.";
  //     return Error{Error::Kind::ArgumentNumberMismatch,
  //     call.sourceLocation(),
  //                  msg.view()};
  //   }

  //   auto cursor = formal_arguments.begin();
  //   for (auto &actual_argument : actual_arguments) {
  //     auto result = typecheck(actual_argument, *ir, *env);
  //     if (!result)
  //       return result;

  //     auto formal_argument_type = *cursor;
  //     if (!type::equals(formal_argument_type, result.value())) {
  //       std::stringstream msg;
  //       msg << "Expected Type: [" << formal_argument_type << "] ";
  //       msg << "Recieved Type: [" << result.value() << "]";
  //       return Error{Error::Kind::ArgumentTypeMismatch,
  //       call.sourceLocation(),
  //                    msg.view()};
  //     }

  //     ++cursor;
  //   }

  //   return call.cachedType(function_type->result_type);
  // }

  // Result<type::Ptr> operator()(ir::Lambda &lambda) noexcept {
  //   if (lambda.cachedType() != nullptr) {
  //     return lambda.cachedType();
  //   }

  //   env->pushScope();

  //   auto &formal_arguments = lambda.arguments();

  //   type::Function::Arguments argument_types;
  //   argument_types.reserve(formal_arguments.size());

  //   for (auto &formal_argument : formal_arguments) {
  //     argument_types.emplace_back(formal_argument.type);
  //     env->declareName(formal_argument);
  //   }

  //   auto result = typecheck(lambda.body(), *env);
  //   if (!result) {
  //     env->popScope();
  //     return result;
  //   }

  //   if (lambda.annotation()) {
  //     auto annotated_type = lambda.annotation().value();
  //     if (!equals(annotated_type, result.value())) {
  //       std::stringstream msg;
  //       msg << "Expected Type [" << annotated_type << "]"
  //           << " Actual Type [" << result.value() << "]";
  //       return Error{Error::Kind::ResultTypeMismatch,
  //       lambda.sourceLocation(),
  //                    msg.view()};
  //     }
  //   }

  //   auto function_type =
  //       env->getFunctionType(result.value(), std::move(argument_types));
  //   auto lambda_type = env->getLambdaType(function_type);

  //   env->popScope();
  //   return lambda.cachedType(lambda_type);
  // }

  Result<type::Ptr> operator()(ir::Import &i) noexcept {
    if (i.cachedType() != nullptr) {
      return i.cachedType();
    }

    if (env->alreadyImported(i.file())) {
      return env->getNilType();
    }

    if (!env->fileExists(i.file())) {
      return Error{Error::Kind::FileNotFound, i.sourceLocation(), i.file()};
    }

    return i.cachedType(env->getNilType());
  }

  Result<type::Ptr> operator()(ir::Module &m) noexcept {
    if (m.cachedType() != nullptr) {
      return m.cachedType();
    }

    env->pushScope(m.name());

    std::size_t index = 0U;
    auto &recovered_expressions = m.recovered_expressions();
    for (auto &expression : m.expressions()) {
      auto result = typecheck(expression, *env);
      if (result.recovered()) {
        recovered_expressions[index] = true;
      } else if (!result) {
        env->unbindScope(m.name());
        env->popScope();
        return result;
      }
      ++index;
    }

    env->popScope();
    return m.cachedType(env->getNilType());
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

int typecheck(Environment &env) noexcept {
  for (auto &expression : env.importedExpressions()) {
    auto result = typecheck(expression, env);
    if (!result) {
      if (result.recovered()) {
        continue;
      }

      env.errorStream() << result.error() << "\n";
      return EXIT_FAILURE;
    }
  }

  for (auto &expression : env.localExpressions()) {
    auto result = typecheck(expression, env);
    if (!result) {
      if (result.recovered()) {
        continue;
      }

      env.errorStream() << result.error() << "\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

} // namespace mint
