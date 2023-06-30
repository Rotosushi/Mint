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
#include "ast/expression/Binop.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Result<type::Ptr> Binop::typecheck(Environment &env) const noexcept {
  auto overloads = env.lookupBinop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownBinop, location(), tokenToView(m_op)};

  auto left_result = m_left->typecheck(env);
  if (!left_result)
    return left_result;
  auto left_type = left_result.value();

  auto right_result = m_right->typecheck(env);
  if (!right_result)
    return right_result;
  auto right_type = right_result.value();

  auto instance = overloads->lookup(left_type, right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of binop [" << m_op
            << "] exists given argument types [" << left_type << ","
            << right_type << "]";
    return {Error::Kind::BinopTypeMismatch, location(), message.view()};
  }

  setCachedType(instance->result_type);
  return instance->result_type;
}

Result<ast::Ptr> Binop::evaluate(Environment &env) noexcept {
  auto overloads = env.lookupBinop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownBinop, location(), tokenToView(m_op)};

  auto left_result = m_left->evaluate(env);
  if (!left_result)
    return left_result;
  auto left_value = left_result.value();
  auto left_type = m_left->cachedTypeOrAssert();

  auto right_result = m_right->evaluate(env);
  if (!right_result)
    return right_result;
  auto right_value = right_result.value();
  auto right_type = m_right->cachedTypeOrAssert();

  auto instance = overloads->lookup(left_type, right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of binop [" << m_op
            << "] exists given argument types [" << left_type << ","
            << right_type << "]";
    return {Error::Kind::BinopTypeMismatch, location(), message.view()};
  }

  return (*instance)(left_value, right_value, env);
}
} // namespace ast
} // namespace mint