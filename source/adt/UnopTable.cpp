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
auto eval_unop_minus(ast::Ast *right, Environment &env) -> Result<ast::Ptr> {
  auto *integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {}, -(integer->value()));
}

auto codegen_unop_minus(llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMNeg(right);
}

auto eval_unop_not(ast::Ast *right, Environment &env) -> Result<ast::Ptr> {
  auto *boolean = llvm::cast<ast::Boolean>(right);
  return env.getBooleanAst({}, {}, !(boolean->value()));
}

auto codegen_unop_not(llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMNot(right);
}

void InitializeBuiltinUnops(Environment *env) {
  auto minus = env->createUnop(Token::Minus);
  minus.emplace(env->getIntegerType(), env->getIntegerType(), eval_unop_minus,
                codegen_unop_minus);

  auto negate = env->createUnop(Token::Not);
  negate.emplace(env->getBooleanType(), env->getBooleanType(), eval_unop_not,
                 codegen_unop_not);
}
} // namespace mint
