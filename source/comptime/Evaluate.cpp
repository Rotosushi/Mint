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
    MINT_ASSERT(binding.hasComptimeValue());

    return binding.comptimeValueOrAssert();
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

    // evaluating a function is as simple as binding the
    // name to the ast representing the function.
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

    auto left_type_result = typecheck(b.left, env);
    if (!left_type_result) {
      return left_type_result.error();
    }
    auto left_type = left_type_result.value();

    auto left_result = evaluate(b.left, env);
    if (!left_result) {
      return left_result;
    }
    auto &left = left_result.value();

    auto right_type_result = typecheck(b.right, env);
    if (!right_type_result) {
      return right_type_result.error();
    }
    auto right_type = right_type_result.value();

    auto right_result = evaluate(b.right, env);
    if (!right_result) {
      return right_result;
    }
    auto &right = right_result.value();

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

    auto right_type_result = typecheck(u.right, env);
    if (!right_type_result) {
      return right_type_result.error();
    }
    auto right_type = right_type_result.value();

    auto right_result = evaluate(u.right, env);
    if (!right_result) {
      return right_result;
    }
    auto &right = right_result.value();

    auto instance = overloads->lookup(right_type);
    if (!instance) {
      std::stringstream msg;
      msg << "Actual Type [" << right_type << "]";
      return Error{Error::Kind::UnopTypeMismatch, ptr->sl, msg.view()};
    }

    return instance->evaluate(right);
  }

  Result<ast::Ptr> operator()(ast::Call &c) noexcept {
    auto callee_type_result = typecheck(c.callee, env);
    MINT_ASSERT(callee_type_result);
    auto callee_type = callee_type_result.value();
    MINT_ASSERT(type::callable(callee_type));

    auto callee_result = evaluate(c.callee, env);
    if (!callee_result) {
      return callee_result;
    }
    auto &callee = callee_result.value();

    auto [args, body] = [&]() {
      if (callee->holds<ast::Function>()) {
        auto &function = callee->get<ast::Function>();
        return std::make_pair(std::ref(function.arguments),
                              std::ref(function.body));
      } else {
        abort("bad callee type.");
      }
    }();

    std::vector<ast::Ptr> actual_arguments;
    actual_arguments.reserve(args.size());
    for (auto &arg : c.arguments) {
      auto result = evaluate(arg, env);
      if (!result) {
        return result;
      }
      actual_arguments.emplace_back(result.value());
    }

    env.pushScope();
    auto actual_args_cursor = actual_arguments.begin();
    for (auto &arg : args) {
      auto result = env.declareName(arg);
      if (!result) {
        env.popScope();
        return result.error();
      }

      auto binding = result.value();
      binding.setComptimeValue(*actual_args_cursor);
      ++actual_args_cursor;
    }

    // #NOTE: an empty function body is treated as if
    // it returns nil.
    // #NOTE: the return value of a function is the
    // last expression in it's body
    ast::Ptr return_value = ast::create();
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
    auto found = env.fileResolve(i.file);
    MINT_ASSERT(found);
    auto &path = found.value();

    auto *itu = env.findImport(path);
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
