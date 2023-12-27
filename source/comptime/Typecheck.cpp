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
#include "comptime/Import.hpp"
#include "comptime/Typecheck.hpp"
#include "utility/Abort.hpp"

namespace mint {
struct Recover {
  Recoverable &recoverable;
  ast::Ptr &ptr;
  Identifier def;
  Environment &env;

  Recover(Recoverable &r, ast::Ptr &p, Identifier d, Environment &e) noexcept
      : recoverable(r), ptr(p), def(d), env(e) {}

  Result<type::Ptr> operator()() noexcept {
    return std::visit(*this, recoverable.data());
  }

  Result<type::Ptr> operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return Error{Error::Kind::Default};
  }

  Result<type::Ptr> operator()(Recoverable::UBD &ubd) noexcept {
    if (auto failed =
            env.bindUseBeforeDef(ubd.undef_name, def, ubd.local_scope, ptr)) {
      return failed.value();
    }

    return Recovered{};
  }
};

Result<type::Ptr> recover(Recoverable &r, ast::Ptr &p, Identifier d,
                          Environment &e) noexcept {
  Recover visitor(r, p, d, e);
  return visitor();
}

struct TypecheckAst {
  Environment &env;
  ast::Ptr &ptr;

  TypecheckAst(Environment &env, ast::Ptr &ptr) noexcept : env(env), ptr(ptr) {}

  Result<type::Ptr> operator()() noexcept {
    return std::visit(*this, ptr->variant);
  }

  // the type of nil is Nil.
  Result<type::Ptr> operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return ptr->setCachedType(env.getNilType());
  }

  // the type of a boolean is Boolean
  Result<type::Ptr> operator()([[maybe_unused]] bool &b) noexcept {
    return ptr->setCachedType(env.getBooleanType());
  }

  // the type of an integer is Integer
  Result<type::Ptr> operator()([[maybe_unused]] int &i) noexcept {
    return ptr->setCachedType(env.getIntegerType());
  }

  // The type of a variable is the type it is bound to in scope.
  Result<type::Ptr> operator()(Identifier &i) noexcept {
    auto result = env.lookupBinding(i);
    if (!result) {
      auto error = result.error();
      if (error.kind() == Error::Kind::NameUnboundInScope) {
        return Recoverable{i, env.nearestNamedScope()};
      }
      return error;
    }

    return ptr->setCachedType(result.value().type());
  }

  // A lambdas type is the type of it's arguments and the
  // type of it's return value. the optional type annotation
  // must match the return type if present
  Result<type::Ptr> operator()(ast::Lambda &l) noexcept {
    env.pushScope();
    type::Function::Arguments arg_types;
    arg_types.reserve(l.arguments.size());
    for (auto &arg : l.arguments) {
      arg_types.emplace_back(arg.type);
      env.declareName(arg);
    }

    // #TODO: support composite lambda bodies
    auto result = typecheck(l.body.front(), env);
    if (!result) {
      env.popScope();
      return result;
    }
    env.popScope();

    if (l.annotation) {
      auto type = l.annotation.value();
      if (!type::equals(type, result.value())) {
        std::stringstream msg;
        msg << "Annotated Type [" << type << "], ";
        msg << "Actual Type [" << result.value() << "]";
        return Error{Error::Kind::AnnotatedTypeMismatch, ptr->sl, msg.view()};
      }
    }

    auto function_type =
        env.getFunctionType(result.value(), std::move(arg_types));
    auto lambda_type = env.getLambdaType(function_type);
    return ptr->setCachedType(lambda_type);
  }

  // A functions type is the type of it's arguments and the type
  // of it's return value. The optional type annotation must
  // match the return type if present.
  Result<type::Ptr> operator()(ast::Function &f) noexcept {
    if (auto found = env.lookupLocalBinding(f.name)) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl, f.name};
    }
    type::Ptr result_type = env.getNilType();
    type::Function::Arguments arg_types;

    env.pushScope();
    if (f.name == env.getIdentifier("main")) {
      if (!f.arguments.empty()) {
        return Error{Error::Kind::MainArgumentTypesMismatch, ptr->sl, ""};
      }

      for (auto &expression : f.body) {
        auto result = typecheck(expression, env);
        if (result.recoverable()) {
          env.popScope();
          return recover(result.unknown(), ptr, env.qualifyName(f.name), env);
        } else if (!result) {
          env.popScope();
          return result;
        }

        result_type = result.value();
      }

      if (!type::equals(result_type, env.getIntegerType())) {
        std::stringstream msg;
        msg << "Actual Type [" << result_type << "]";
        return Error{Error::Kind::MainReturnTypeMismatch, ptr->sl, msg.view()};
      }

      if (f.annotation &&
          !type::equals(f.annotation.value(), env.getIntegerType())) {
        std::stringstream msg;
        msg << "Annotated Type [" << f.annotation.value() << "]";
        return Error{Error::Kind::MainAnnotatedTypeMismatch, ptr->sl,
                     msg.view()};
      }

      result_type = env.getNilType();
    } else {
      arg_types.reserve(f.arguments.size());
      for (auto &arg : f.arguments) {
        arg_types.emplace_back(arg.type);
        env.declareName(arg);
      }

      // #TODO: add an explicit return statement
      // #NOTE: the final term in the body is assumed
      // to be the result type.
      // #NOTE: if there are zero expressions in the body,
      // it is as if the body only contains the nil constant.
      for (auto &expression : f.body) {
        auto result = typecheck(expression, env);
        if (result.recoverable()) {
          env.popScope();
          return recover(result.unknown(), ptr, env.qualifyName(f.name), env);
        } else if (!result) {
          env.popScope();
          return result;
        }

        result_type = result.value();
      }

      if (f.annotation) {
        auto type = f.annotation.value();
        if (!type::equals(type, result_type)) {
          std::stringstream msg;
          msg << "Annotated Type [" << type << "], ";
          msg << "Actual Type [" << result_type << "]";
          return Error{Error::Kind::AnnotatedTypeMismatch, ptr->sl, msg.view()};
        }
      }
    }
    env.popScope();

    auto function_type = env.getFunctionType(result_type, std::move(arg_types));

    if (auto bound = env.declareName(f.name, f.attributes, function_type);
        !bound) {
      return bound.error();
    }

    if (auto failed = env.resolveTypeOfUseBeforeDef(env.qualifyName(f.name))) {
      return failed.value();
    }

    return ptr->setCachedType(function_type);
  }

  // A let expression's type is inferred to be the type of
  // the expression it is binding. The optional type annotation
  // must match the inferred type if present.
  Result<type::Ptr> operator()(ast::Let &l) noexcept {
    if (auto found = env.lookupLocalBinding(l.name)) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl, l.name};
    }

    // If the expression fails to typecheck due to
    // a name not having a definition, then we can
    // delay failing to typecheck this let expression
    // by using the UBD machinery. Only definitions
    // can be delayed using the UBD machinery.
    // (currently, let and functions are the only definitions)
    auto result = typecheck(l.affix, env);
    if (result.recoverable()) {
      return recover(result.unknown(), ptr, env.qualifyName(l.name), env);
    } else if (!result) {
      return result;
    }

    if (l.annotation) {
      auto type = l.annotation.value();
      if (!type::equals(type, result.value())) {
        std::stringstream msg;
        msg << "Annotated Type [" << type << "], ";
        msg << "Actual Type [" << result.value() << "]";
        return Error{Error::Kind::AnnotatedTypeMismatch, ptr->sl, msg.view()};
      }
    }

    if (auto bound = env.declareName(l.name, l.attributes, result.value());
        !bound) {
      return bound.error();
    }
    // since we just declared the type of a name, we can
    // attempt to resolve the types of any names which
    // depend on this name.
    if (auto failed = env.resolveTypeOfUseBeforeDef(env.qualifyName(l.name))) {
      return failed.value();
    }

    return ptr->setCachedType(env.getNilType());
  }

  Result<type::Ptr> operator()(ast::Binop &b) noexcept {
    auto overloads = env.lookupBinop(b.op);
    if (!overloads) {
      return Error{Error::Kind::UnknownBinop, ptr->sl, tokenToView(b.op)};
    }

    auto left_result = typecheck(b.left, env);
    if (!left_result) {
      return left_result;
    }
    auto left = left_result.value();

    auto right_result = typecheck(b.right, env);
    if (!right_result) {
      return right_result;
    }
    auto right = right_result.value();

    auto instance = overloads->lookup(left, right);
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Types [" << left << ", " << right << "]";
      return Error{Error::Kind::BinopTypeMismatch, ptr->sl, msg.view()};
    }

    return ptr->setCachedType(instance->result_type);
  }

  Result<type::Ptr> operator()(ast::Unop &u) noexcept {
    auto overloads = env.lookupUnop(u.op);
    if (!overloads) {
      return Error{Error::Kind::UnknownUnop, ptr->sl, tokenToView(u.op)};
    }

    auto right_result = typecheck(u.right, env);
    if (!right_result) {
      return right_result;
    }
    auto right = right_result.value();

    auto instance = overloads->lookup(right);
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Type [" << right << "]";
      return Error{Error::Kind::UnopTypeMismatch, ptr->sl, msg.view()};
    }

    return ptr->setCachedType(instance->result_type);
  }

  Result<type::Ptr> operator()(ast::Call &c) noexcept {
    auto callee_result = typecheck(c.callee, env);
    if (!callee_result) {
      return callee_result;
    }
    auto callee = callee_result.value();

    if (!type::callable(callee)) {
      std::stringstream msg;
      msg << "Callee Type [" << callee << "]";
      return Error{Error::Kind::CannotCallType, ptr->sl, msg.view()};
    }

    type::Function *function_type = [&]() {
      if (callee->holds<type::Lambda>()) {
        auto &lambda_type = callee->get<type::Lambda>();
        return &lambda_type.function_type->get<type::Function>();
      } else if (callee->holds<type::Function>()) {
        return &callee->get<type::Function>();
      } else {
        abort("bad callee type.");
      }
    }();

    auto &actual_arguments = c.arguments;
    auto &formal_arguments = function_type->arguments;
    if (actual_arguments.size() != formal_arguments.size()) {
      std::stringstream msg;
      msg << "Expected [" << formal_arguments.size() << "] arguments, ";
      msg << "Have [" << actual_arguments.size() << "]";
      return Error{Error::Kind::ArgumentNumberMismatch, ptr->sl, msg.view()};
    }

    auto formal_cursor = formal_arguments.begin();
    auto formal_end = formal_arguments.end();
    auto actual_cursor = actual_arguments.begin();
    while (formal_cursor != formal_end) {
      auto &formal_argument_type = *formal_cursor;
      auto &actual_argument = *actual_cursor;

      auto result = typecheck(actual_argument, env);
      if (!result) {
        return result;
      }
      auto actual_argument_type = result.value();

      if (!type::equals(formal_argument_type, actual_argument_type)) {
        std::stringstream msg;
        msg << "Expected Type [" << formal_argument_type << "], ";
        msg << "Actual Type [" << actual_argument_type << "]";
        return Error{Error::Kind::ArgumentTypeMismatch, ptr->sl, msg.view()};
      }

      ++formal_cursor;
      ++actual_cursor;
    }

    return ptr->setCachedType(function_type->result_type);
  }

  Result<type::Ptr> operator()(ast::Parens &p) noexcept {
    return typecheck(p.expression, env);
  }

  Result<type::Ptr> operator()(ast::Import &i) noexcept {
    fs::path path = i.file;

    if (env.alreadyImported(path)) {
      return env.getNilType();
    }

    if (!env.fileExists(path)) {
      return Error{Error::Kind::FileNotFound, ptr->sl, i.file};
    }

    if (importSourceFile(path, env) == EXIT_FAILURE) {
      return Error{Error::Kind::ImportFailed, ptr->sl, i.file};
    }

    auto *itu = env.findImport(path);
    MINT_ASSERT(itu != nullptr);

    std::vector<TranslationUnit::Expressions::iterator> recovered_expressions;
    auto &local_expressions = itu->expressions();
    auto cursor = local_expressions.begin();
    auto end = local_expressions.end();
    while (cursor != end) {
      auto &expression = *cursor;
      auto result = typecheck(expression, env);
      if (result.recovered()) {
        recovered_expressions.emplace_back(cursor);
      } else if (!result) {
        env.errorStream() << result.error() << "\n";
        return Error{Error::Kind::ImportFailed, ptr->sl, i.file};
      }

      ++cursor;
    }

    // remove recovered expressions from the local_expressions,
    // as these expressions are held in the UseBeforeDefMap, and
    // we do not want to process them twice.
    // #NOTE: this requires that iterators into the local_expressions
    // remain stable when erasing one or more elements.
    for (auto iter : recovered_expressions) {
      local_expressions.erase(iter);
    }

    return ptr->setCachedType(env.getNilType());
  }

  Result<type::Ptr> operator()(ast::Module &m) noexcept {
    env.pushScope(m.name);
    std::vector<ast::Module::Expressions::iterator> recovered_expressions;
    auto &local_expressions = m.expressions;
    auto cursor = local_expressions.begin();
    auto end = local_expressions.end();
    while (cursor != end) {
      auto &expression = *cursor;
      auto result = typecheck(expression, env);
      if (result.recovered()) {
        recovered_expressions.emplace_back(cursor);
      } else if (!result) {
        env.unbindScope(m.name);
        env.popScope();
        return result;
      }

      ++cursor;
    }
    env.popScope();

    // remove recovered expressions from the local_expressions,
    // as these expressions are held in the UseBeforeDefMap, and
    // we do not want to process them twice.
    // #NOTE: this requires that iterators into the local_expressions
    // remain stable when erasing one or more elements.
    for (auto iter : recovered_expressions) {
      local_expressions.erase(iter);
    }

    return ptr->setCachedType(env.getNilType());
  }
};

Result<type::Ptr> typecheck(ast::Ptr &ptr, Environment &env) noexcept {
  if (ptr->cached_type) {
    return ptr->cached_type.value();
  }

  TypecheckAst visitor(env, ptr);
  return visitor();
}

int typecheck(Environment &env) noexcept {
  std::vector<TranslationUnit::Expressions::iterator> recovered_expressions;
  auto &local_expressions = env.localExpressions();
  auto cursor = local_expressions.begin();
  auto end = local_expressions.end();
  while (cursor != end) {
    auto &expression = *cursor;
    auto result = typecheck(expression, env);
    if (result.recovered()) {
      recovered_expressions.emplace_back(cursor);
      // [[fallthrough]]
    } else if (!result) {
      env.errorStream() << result.error() << "\n";
      return EXIT_FAILURE;
    } // else (result.success()) { [[fallthrough]] }

    ++cursor;
  }

  // remove recovered expressions from the local_expressions,
  // as these expressions are held in the UseBeforeDefMap, and
  // we do not want to process them twice.
  // #NOTE: this requires that iterators into the local_expressions
  // remain stable when erasing one or more elements.
  for (auto iter : recovered_expressions) {
    local_expressions.erase(iter);
  }

  return EXIT_SUCCESS;
}
} // namespace mint
