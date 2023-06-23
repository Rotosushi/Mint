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
#pragma once
#include <sstream>

#include "adt/Environment.hpp"
#include "ast/Ast.hpp"
#include "ast/Clone.hpp"
#include "ast/Typecheck.hpp"
#include "error/Result.hpp"

namespace mint {
[[nodiscard]] auto evaluate(Ast::Ptr const &ast, Environment *env)
    -> Result<Ast::Ptr>;

class AstEvaluateVisitor {
  Ast::Ptr ast;
  Environment *env;

public:
  AstEvaluateVisitor(Ast::Ptr ast, Environment *env) noexcept
      : ast(ast), env(env) {
    MINT_ASSERT(this->ast != nullptr);
    MINT_ASSERT(this->env != nullptr);
  }

  auto operator()() noexcept -> Result<Ast::Ptr> {
    return std::visit(*this, ast->data);
  }

  auto operator()(Ast::Let &let) noexcept -> Result<Ast::Ptr> {
    auto value = evaluate(let.term, env);
    if (!value)
      return value;

    auto type_result = typecheck(let.term, env);
    if (!type_result)
      return std::move(type_result.error());
    auto type = type_result.value();

    env->bindName(let.id, let.attributes, type, clone(value.value(), env));
    return env->getNilAst({}, let.location);
  }

  auto operator()(Ast::Module &m) noexcept -> Result<Ast::Ptr> {
    env->pushScope(m.name);

    for (auto &expr : m.expressions) {
      auto result = evaluate(expr, env);
      if (!result) {
        env->unbindScope(m.name);
        env->popScope();
        return result;
      }
    }

    env->popScope();

    return env->getNilAst({}, m.location);
  }

  auto operator()(Ast::Import &i) noexcept -> Result<Ast::Ptr> {
    auto file_found = env->fileSearch(i.file);
    if (!file_found) {
      return {Error::FileNotFound, i.location, i.file};
    }
    auto &file = file_found.value();
    Parser parser{env, &file};

    while (!parser.endOfInput()) {
      auto parse_result = parser.parse();
      if (!parse_result) {
        auto &error = parse_result.error();
        if (error.getKind() == Error::EndOfInput)
          break;
        env->printErrorWithSource(error, parser);
        return {Error::ImportFailed, i.location, i.file};
      }
      auto &ast = parse_result.value();

      auto typecheck_result = typecheck(ast, env);
      if (!typecheck_result) {
        env->printErrorWithSource(typecheck_result.error(), parser);
        return {Error::ImportFailed, i.location, i.file};
      }

      auto evaluate_result = evaluate(ast, env);
      if (!evaluate_result) {
        env->printErrorWithSource(evaluate_result.error(), parser);
        return {Error::ImportFailed, i.location, i.file};
      }
    }

    return env->getNilAst({}, i.location);
  }

  auto operator()(Ast::Binop &binop) noexcept -> Result<Ast::Ptr> {
    auto overloads = env->lookupBinop(binop.op);
    if (!overloads) {
      return {Error::UnknownBinop, binop.location, toString(binop.op)};
    }

    auto left_value = evaluate(binop.left, env);
    if (!left_value) {
      return left_value;
    }

    auto left_type = typecheck(binop.left, env);
    if (!left_type) {
      return std::move(left_type.error());
    }

    auto right_value = evaluate(binop.right, env);
    if (!right_value) {
      return right_value;
    }

    auto right_type = typecheck(binop.right, env);
    if (!right_type) {
      return std::move(right_type.error());
    }

    auto instance = overloads->lookup(left_type.value(), right_type.value());
    if (!instance) {
      std::stringstream ss;
      ss << "[" << left_type.value() << ", " << right_type.value() << "]";
      return {Error::BinopTypeMismatch, binop.location, ss.view()};
    }

    return instance.value()(left_value.value().get(), right_value.value().get(),
                            env);
  }

  auto operator()(Ast::Unop &unop) noexcept -> Result<Ast::Ptr> {
    auto overloads = env->lookupUnop(unop.op);
    if (!overloads) {
      return {Error::UnknownUnop, unop.location, toString(unop.op)};
    }

    auto right_value = evaluate(unop.right, env);
    if (!right_value) {
      return right_value;
    }

    auto right_type = typecheck(unop.right, env);
    if (!right_type) {
      return std::move(right_type.error());
    }

    auto instance = overloads->lookup(right_type.value());
    if (!instance) {
      std::stringstream ss;
      ss << "[" << right_type.value() << "]";
      return {Error::UnopTypeMismatch, ast_location(unop.right),
              toString(unop.op)};
    }

    return instance.value()(right_value.value().get(), env);
  }

  auto operator()(Ast::Term &term) noexcept -> Result<Ast::Ptr> {
    if (term.ast.has_value()) {
      return evaluate(term.ast.value(), env);
    }
    return env->getNilAst({}, {});
  }

  auto operator()(Ast::Parens &parens) noexcept -> Result<Ast::Ptr> {
    return evaluate(parens.ast, env);
  }

  auto operator()(Ast::Variable &variable) noexcept -> Result<Ast::Ptr> {
    auto bound = env->lookup(variable.name);
    if (!bound) {
      return {bound.error().getKind(), variable.location, variable.name.view()};
    }

    return {bound.value().value()};
  }

  auto operator()([[maybe_unused]] Ast::Value &value) noexcept
      -> Result<Ast::Ptr> {
    return ast;
  }
};

[[nodiscard]] auto evaluate(Ast::Ptr const &ast, Environment *env)
    -> Result<Ast::Ptr> {
  AstEvaluateVisitor visitor{ast, env};
  return visitor();
}
} // namespace mint
