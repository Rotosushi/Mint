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
#include "adt/UnopTable.hpp"
#include "adt/Environment.hpp"

namespace mint {
auto unop_minus(ast::Ptr &right, Environment &env) -> Result<ast::Ptr> {}

auto unop_not(ast::Ptr &right, Environment &env) -> Result<ast::Ptr> {}

void InitializeBuiltinUnops(Environment *env) {
  auto minus = env->createUnop(Token::Minus);
  minus.emplace(env->getIntegerType(), env->getIntegerType(), unop_minus);

  auto negate = env->createUnop(Token::Not);
  negate.emplace(env->getBooleanType(), env->getBooleanType(), unop_not);
}
} // namespace mint
