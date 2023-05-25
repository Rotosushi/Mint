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
auto binop_add(Ast *left, Ast *right, Environment *env)
    -> Result<Ast::Value *> {
  auto left_value = get<Ast::Value>(left);
  auto left_integer = get<Ast::Value::Integer>(left_value);
  auto right_value = get<Ast::Value>(right);
  auto right_integer = get<Ast::Value::Integer>(right_value);
  return get<Ast::Value>(
      env->getIntegerAst({}, left_integer->value + right_integer->value));
}

void InitializeBuiltinBinops(Environment *env) {
  auto integer_type = env->getIntegerType();

  auto plus = env->createBinop(Token::Plus);
  plus.emplace(integer_type, integer_type, integer_type, binop_add);
}

} // namespace mint
