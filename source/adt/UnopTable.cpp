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
#include "ast/value/Boolean.hpp"
#include "ast/value/Integer.hpp"

namespace mint {
[[nodiscard]] auto UnopOverload::evaluate(ir::Scalar right) -> ir::Scalar {
  return eval(right);
}
[[nodiscard]] auto UnopOverload::codegen(llvm::Value *right, Environment &env)
    -> llvm::Value * {
  return gen(right, env);
}

auto UnopOverloads::lookup(type::Ptr right_type) noexcept
    -> std::optional<UnopOverload> {
  for (auto &overload : overloads) {
    if (right_type == overload.right_type) {
      return overload;
    }
  }
  return std::nullopt;
}

auto UnopOverloads::emplace(type::Ptr right_type, type::Ptr result_type,
                            UnopEvalFn eval, UnopCodegenFn codegen) noexcept
    -> UnopOverload {
  auto found = lookup(right_type);
  if (found) {
    return found.value();
  }

  return overloads.emplace_back(right_type, result_type, eval, codegen);
}

UnopTable::Unop::Unop(Table::iterator iter) noexcept : iter(iter) {}

auto UnopTable::Unop::lookup(type::Ptr right_type) noexcept
    -> std::optional<UnopOverload> {
  return iter->second.lookup(right_type);
}

auto UnopTable::Unop::emplace(type::Ptr right_type, type::Ptr result_type,
                              UnopEvalFn eval, UnopCodegenFn codegen) noexcept
    -> UnopOverload {
  return iter->second.emplace(right_type, result_type, eval, codegen);
}

auto UnopTable::lookup(Token op) noexcept -> std::optional<Unop> {
  auto found = table.find(op);
  if (found != table.end()) {
    return found;
  }
  return std::nullopt;
}

auto UnopTable::emplace(Token op) noexcept -> Unop {
  auto found = table.find(op);
  if (found != table.end()) {
    return found;
  }

  return table.emplace(op, UnopOverloads{}).first;
}

auto eval_unop_minus(ir::Scalar right) -> ir::Scalar {
  MINT_ASSERT(right.holds<int>());
  return {-right.get<int>()};
}

auto codegen_unop_minus(llvm::Value *right, Environment &env) -> llvm::Value * {
  return env.createLLVMNeg(right);
}

auto eval_unop_not(ir::Scalar right) -> ir::Scalar {
  MINT_ASSERT(right.holds<bool>());
  return {!right.get<bool>()};
}

auto codegen_unop_not(llvm::Value *right, Environment &env) -> llvm::Value * {
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
