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
#include "ast/expression/Unop.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Unop::Unop(Attributes attributes, Location location, Token op,
           Ptr right) noexcept
    : Expression{Ast::Kind::Unop, attributes, location}, m_op{op},
      m_right{std::move(right)} {
  m_right->setPrevAst(this);
}

[[nodiscard]] auto Unop::create(Attributes attributes, Location location,
                                Token op, Ptr right) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Unop>(attributes, location, op, std::move(right)));
}

auto Unop::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Unop;
}

Ptr Unop::clone() const noexcept {
  return create(attributes(), location(), m_op, m_right->clone());
}

void Unop::print(std::ostream &out) const noexcept {
  out << m_op << " " << m_right;
}

Result<type::Ptr> Unop::typecheck(Environment &env) const noexcept {
  auto overloads = env.lookupUnop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownUnop, location(), tokenToView(m_op)};

  auto right_result = m_right->typecheck(env);
  if (!right_result)
    return right_result;
  auto right_type = right_result.value();

  auto instance = overloads->lookup(right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of [" << m_op << "] found for type [" << right_type
            << "]";
    return {Error::Kind::UnopTypeMismatch, m_right->location(), message.view()};
  }

  return setCachedType(instance->result_type);
}

Result<ast::Ptr> Unop::evaluate(Environment &env) noexcept {
  auto overloads = env.lookupUnop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownUnop, location(), tokenToView(m_op)};

  auto right_type = m_right->cachedTypeOrAssert();

  auto right_result = m_right->evaluate(env);
  if (!right_result)
    return right_result;
  auto &right_value = right_result.value();

  auto instance = overloads->lookup(right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of [" << m_op << "] found for type [" << right_type
            << "]";
    return {Error::Kind::UnopTypeMismatch, m_right->location(), message.view()};
  }

  return instance->evaluate(right_value.get());
}

Result<llvm::Value *> Unop::codegen(Environment &env) noexcept {
  auto overloads = env.lookupUnop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownUnop, location(), tokenToView(m_op)};

  auto right_type = m_right->cachedTypeOrAssert();

  auto right_result = m_right->codegen(env);
  if (!right_result)
    return right_result;
  auto right_value = right_result.value();

  auto instance = overloads->lookup(right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of [" << m_op << "] found for type [" << right_type
            << "]";
    return {Error::Kind::UnopTypeMismatch, m_right->location(), message.view()};
  }

  return instance->codegen(right_value, env);
}
} // namespace ast
} // namespace mint