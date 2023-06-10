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

#include "ast/Ast.hpp"
#include "ast/Typecheck.hpp"

#include "adt/Environment.hpp"

#include "error/Error.hpp"

namespace mint {
using EvaluateResult = Result<Ast::Pointer>;

[[nodiscard]] auto evaluate(Ast::Pointer const &ast, Environment *env)
    -> EvaluateResult;

class AstEvaluateVisitor {
  Ast::Pointer ast;
  Environment *env;

public:
  AstEvaluateVisitor(Ast::Pointer ast, Environment *env) noexcept
      : ast(ast), env(env) {
    MINT_ASSERT(this->ast != nullptr);
    MINT_ASSERT(this->env != nullptr);
  }

  auto operator()() noexcept -> EvaluateResult {
    return std::visit(*this, ast->data);
  }

  auto operator()(Ast::Term &affix) noexcept -> EvaluateResult {
    return evaluate(affix.affix, env);
  }

  auto operator()([[maybe_unused]] Ast::Type &type) noexcept -> EvaluateResult {
    return ast;
  }

  auto operator()(Ast::Let &let) noexcept -> EvaluateResult {
    auto value = evaluate(let.term, env);
    if (!value)
      return value;

    auto type = typecheck(let.term, env);
    if (!type)
      return std::move(type.error());

    env->bind(let.id, let.attributes, type.value(), value.value());
    return value.value();
  }

  auto operator()(Ast::Binop &binop) noexcept -> EvaluateResult {
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

  auto operator()(Ast::Unop &unop) noexcept -> EvaluateResult {
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

  auto operator()(Ast::Parens &parens) noexcept -> EvaluateResult {
    return evaluate(parens.ast, env);
  }

  auto operator()(Ast::Variable &variable) noexcept -> EvaluateResult {
    auto bound = env->lookup(variable.name);
    if (!bound) {
      return {Error::NameUnboundInScope, variable.location,
              variable.name.view()};
    }

    return {bound.value().value()};
  }

  auto operator()([[maybe_unused]] Ast::Value &value) noexcept
      -> EvaluateResult {
    return ast;
  }
};

[[nodiscard]] auto evaluate(Ast::Pointer const &ast, Environment *env)
    -> EvaluateResult {
  AstEvaluateVisitor visitor{ast, env};
  return visitor();
}
} // namespace mint
