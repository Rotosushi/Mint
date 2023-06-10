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

#include "adt/Environment.hpp"

#include "error/Error.hpp"

namespace mint {
class AstValueTypecheckVisitor {
  Environment *env;

public:
  AstValueTypecheckVisitor(Environment *env) noexcept : env(env) {
    MINT_ASSERT(env != nullptr);
  }

  auto operator()(Ast::Value const &value) noexcept -> Result<Type::Pointer> {
    return std::visit(*this, value.data);
  }

  auto operator()([[maybe_unused]] Ast::Value::Boolean const &boolean) noexcept
      -> Result<Type::Pointer> {
    return env->getBooleanType();
  }

  auto operator()([[maybe_unused]] Ast::Value::Integer const &integer) noexcept
      -> Result<Type::Pointer> {
    return env->getIntegerType();
  }

  auto operator()([[maybe_unused]] Ast::Value::Nil const &nil) noexcept
      -> Result<Type::Pointer> {
    return env->getNilType();
  }
};

[[nodiscard]] auto typecheck(Ast::Value const &value, Environment &env) noexcept
    -> Result<Type::Pointer> {
  AstValueTypecheckVisitor visitor{&env};
  return visitor(value);
}

class AstTypecheckVisitor {
  Environment *env;

public:
  AstTypecheckVisitor(Environment *env) noexcept : env(env) {
    MINT_ASSERT(env != nullptr);
  }

  auto operator()(Ast::Pointer const &ast) noexcept -> Result<Type::Pointer> {
    return std::visit(*this, ast->data);
  }

  auto operator()(Ast::Term const &affix) noexcept -> Result<Type::Pointer> {
    return std::visit(*this, affix.affix->data);
  }

  auto operator()(Ast::Type const &type) noexcept -> Result<Type::Pointer> {
    return {type.type};
  }

  auto operator()(Ast::Let const &let) noexcept -> Result<Type::Pointer> {
    return std::visit(*this, let.term->data);
  }

  auto operator()(Ast::Binop const &binop) noexcept -> Result<Type::Pointer> {
    auto overloads = env->lookupBinop(binop.op);
    if (!overloads) {
      return {Error::UnknownBinop, binop.location, toString(binop.op)};
    }

    auto left_type = std::visit(*this, binop.left->data);
    if (!left_type)
      return left_type;

    auto right_type = std::visit(*this, binop.right->data);
    if (!right_type)
      return right_type;

    auto instance = overloads->lookup(left_type.value(), right_type.value());
    if (!instance) {
      std::stringstream ss;
      ss << "[" << left_type.value() << ", " << right_type.value() << "]";
      return {Error::BinopTypeMismatch, binop.location, ss.view()};
    }

    return instance->result_type;
  }

  auto operator()(Ast::Unop const &unop) noexcept -> Result<Type::Pointer> {
    auto overloads = env->lookupUnop(unop.op);
    if (!overloads) {
      return {Error::UnknownUnop, unop.location, toString(unop.op)};
    }

    auto right_type = std::visit(*this, unop.right->data);
    if (!right_type)
      return right_type;

    auto instance = overloads->lookup(right_type.value());
    if (!instance) {
      std::stringstream ss;
      ss << "[" << right_type.value() << "]";
      return {Error::UnopTypeMismatch, ast_location(unop.right),
              toString(unop.op)};
    }

    return instance->result_type;
  }

  auto operator()(Ast::Parens const &parens) noexcept -> Result<Type::Pointer> {
    return std::visit(*this, parens.ast->data);
  }

  auto operator()(Ast::Value const &value) noexcept -> Result<Type::Pointer> {
    return typecheck(value, *env);
  }

  auto operator()(Ast::Variable &variable) noexcept -> Result<Type::Pointer> {
    auto binding = env->lookup(variable.name);
    if (!binding) {
      return {Error::NameUnboundInScope, variable.location, variable.name.view()};
    }

    return binding.value().type();
  }
};

/*
  #TODO: typecheck doesn't record variables type for typing
  expressions including those variables later in the same scope.
*/
[[nodiscard]] auto typecheck(Ast::Pointer const &ast, Environment *env)
    -> Result<Type::Pointer> {
  auto cache = ast->cached_type();
  if (cache) {
    return cache.value();
  }

  AstTypecheckVisitor visitor{env};
  return visitor(ast);
}
} // namespace mint
