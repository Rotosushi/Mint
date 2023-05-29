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
#include "adt/BinopTable.hpp"
#include "adt/Environment.hpp"

namespace mint {
auto binop_add(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getIntegerAst({}, left_integer->value + right_integer->value);
}

auto binop_sub(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getIntegerAst({}, left_integer->value - right_integer->value);
}

auto binop_mult(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getIntegerAst({}, left_integer->value * right_integer->value);
}

auto binop_div(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getIntegerAst({}, left_integer->value / right_integer->value);
}

auto binop_mod(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getIntegerAst({}, left_integer->value % right_integer->value);
}

auto binop_and(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_boolean = get_value<Ast::Value::Boolean>(left);
  auto right_boolean = get_value<Ast::Value::Boolean>(right);
  return env->getBooleanAst({}, left_boolean->value && right_boolean->value);
}

auto binop_or(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_boolean = get_value<Ast::Value::Boolean>(left);
  auto right_boolean = get_value<Ast::Value::Boolean>(right);
  return env->getBooleanAst({}, left_boolean->value || right_boolean->value);
}

auto binop_integer_equality(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value == right_integer->value);
}

auto binop_boolean_equality(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_boolean = get_value<Ast::Value::Boolean>(left);
  auto right_boolean = get_value<Ast::Value::Boolean>(right);
  return env->getBooleanAst({}, left_boolean->value == right_boolean->value);
}

auto binop_integer_inequality(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value != right_integer->value);
}

auto binop_boolean_inequality(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_boolean = get_value<Ast::Value::Boolean>(left);
  auto right_boolean = get_value<Ast::Value::Boolean>(right);
  return env->getBooleanAst({}, left_boolean->value != right_boolean->value);
}

auto binop_less_than(Ast *left, Ast *right, Environment *env) -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value < right_integer->value);
}

auto binop_less_than_or_equal(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value <= right_integer->value);
}

auto binop_greater_than(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value > right_integer->value);
}

auto binop_greater_than_or_equal(Ast *left, Ast *right, Environment *env)
    -> Result<Ast *> {
  auto left_integer = get_value<Ast::Value::Integer>(left);
  auto right_integer = get_value<Ast::Value::Integer>(right);
  return env->getBooleanAst({}, left_integer->value >= right_integer->value);
}

void InitializeBuiltinBinops(Environment *env) {
  auto integer_type = env->getIntegerType();
  auto boolean_type = env->getBooleanType();

  auto plus = env->createBinop(Token::Plus);
  plus.emplace(integer_type, integer_type, integer_type, binop_add);

  auto minus = env->createBinop(Token::Minus);
  minus.emplace(integer_type, integer_type, integer_type, binop_sub);

  auto mult = env->createBinop(Token::Star);
  mult.emplace(integer_type, integer_type, integer_type, binop_mult);

  auto div = env->createBinop(Token::Divide);
  div.emplace(integer_type, integer_type, integer_type, binop_div);

  auto mod = env->createBinop(Token::Modulo);
  mod.emplace(integer_type, integer_type, integer_type, binop_mod);

  auto boolean_and = env->createBinop(Token::And);
  boolean_and.emplace(boolean_type, boolean_type, boolean_type, binop_and);

  auto boolean_or = env->createBinop(Token::Or);
  boolean_or.emplace(boolean_type, boolean_type, boolean_type, binop_or);

  auto equality = env->createBinop(Token::EqualEqual);
  equality.emplace(integer_type, integer_type, boolean_type,
                   binop_integer_equality);
  equality.emplace(boolean_type, boolean_type, boolean_type,
                   binop_boolean_equality);

  auto inequality = env->createBinop(Token::NotEqual);
  inequality.emplace(integer_type, integer_type, boolean_type,
                     binop_integer_inequality);
  inequality.emplace(boolean_type, boolean_type, boolean_type,
                     binop_boolean_inequality);

  auto less = env->createBinop(Token::LessThan);
  less.emplace(integer_type, integer_type, boolean_type, binop_less_than);

  auto less_or_equal = env->createBinop(Token::LessThanOrEqual);
  less_or_equal.emplace(integer_type, integer_type, boolean_type,
                        binop_less_than_or_equal);

  auto greater = env->createBinop(Token::GreaterThan);
  greater.emplace(integer_type, integer_type, boolean_type, binop_greater_than);

  auto greater_or_equal = env->createBinop(Token::GreaterThanOrEqual);
  greater_or_equal.emplace(integer_type, integer_type, boolean_type,
                           binop_greater_than_or_equal);
}

} // namespace mint
