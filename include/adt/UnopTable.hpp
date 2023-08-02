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
#include <optional>
#include <unordered_map>
#include <vector>

#include "ast/Ast.hpp"
#include "error/Result.hpp"
#include "scan/Token.hpp"

#include "llvm/IR/Value.h"

namespace mint {
class Environment;

using UnopEvalFn = Result<ast::Ptr> (*)(ast::Ast *right, Environment &env);
using UnopCodegenFn = Result<llvm::Value *> (*)(llvm::Value *right,
                                                Environment &env);

struct UnopOverload {
  type::Ptr right_type;
  type::Ptr result_type;
  UnopEvalFn eval;
  UnopCodegenFn gen;

  [[nodiscard]] auto evaluate(ast::Ast *right, Environment &env)
      -> Result<ast::Ptr>;
  [[nodiscard]] auto codegen(llvm::Value *right, Environment &env)
      -> Result<llvm::Value *>;
};

class UnopOverloads {
  std::vector<UnopOverload> overloads;

public:
  UnopOverloads() noexcept { overloads.reserve(2); }

  auto lookup(type::Ptr right_type) noexcept -> std::optional<UnopOverload>;

  auto emplace(type::Ptr right_type, type::Ptr result_type, UnopEvalFn eval,
               UnopCodegenFn codegen) noexcept -> UnopOverload;
};

class UnopTable {
public:
  using Key = Token;
  using Value = UnopOverloads;
  using Pair = std::pair<const Key, Value>;
  using Table = std::unordered_map<Key, Value>;

  class Unop {
    Table::iterator iter;

  public:
    Unop(Table::iterator iter) noexcept;

    auto lookup(type::Ptr right_type) noexcept -> std::optional<UnopOverload>;
    auto emplace(type::Ptr right_type, type::Ptr result_type, UnopEvalFn eval,
                 UnopCodegenFn codegen) noexcept -> UnopOverload;
  };

private:
  Table table;

public:
  auto lookup(Token op) noexcept -> std::optional<Unop>;
  auto emplace(Token op) noexcept -> Unop;
};

void InitializeBuiltinUnops(Environment *env);
} // namespace mint
