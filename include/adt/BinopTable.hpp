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

namespace mint {
class Environment;

using BinopEvalFn = Result<ast::Ptr> (*)(ast::Ast *left, ast::Ast *right,
                                         Environment &env);
using BinopCodegenFn = Result<llvm::Value *> (*)(llvm::Value *left,
                                                 llvm::Value *right,
                                                 Environment &env);

struct BinopOverload {
  type::Ptr left_type;
  type::Ptr right_type;
  type::Ptr result_type;
  BinopEvalFn eval;
  BinopCodegenFn gen;

  [[nodiscard]] auto evaluate(ast::Ast *left, ast::Ast *right,
                              Environment &env) {
    return eval(left, right, env);
  }

  [[nodiscard]] auto codegen(llvm::Value *left, llvm::Value *right,
                             Environment &env) {
    return gen(left, right, env);
  }
};

class BinopOverloads {
  std::vector<BinopOverload> overloads;

public:
  BinopOverloads() noexcept { overloads.reserve(2); }

  auto lookup(type::Ptr left_type, type::Ptr right_type) noexcept
      -> std::optional<BinopOverload> {
    for (auto &overload : overloads) {
      if (left_type == overload.left_type &&
          right_type == overload.right_type) {
        return overload;
      }
    }
    return std::nullopt;
  }

  auto emplace(type::Ptr left_type, type::Ptr right_type, type::Ptr result_type,
               BinopEvalFn eval, BinopCodegenFn gen) noexcept -> BinopOverload {
    auto found = lookup(left_type, right_type);
    if (found) {
      return found.value();
    }

    return overloads.emplace_back(left_type, right_type, result_type, eval,
                                  gen);
  }
};

class BinopTable {
public:
  using Key = Token;
  using Value = BinopOverloads;
  using Pair = std::pair<const Key, Value>;
  using Table = std::unordered_map<Key, Value>;

  class Binop {
    Table::iterator iter;

  public:
    Binop(Table::iterator iter) noexcept : iter(iter) {}

    auto lookup(type::Ptr left_type, type::Ptr right_type) noexcept
        -> std::optional<BinopOverload> {
      return iter->second.lookup(left_type, right_type);
    }

    auto emplace(type::Ptr left_type, type::Ptr right_type,
                 type::Ptr result_type, BinopEvalFn eval,
                 BinopCodegenFn gen) noexcept -> BinopOverload {
      return iter->second.emplace(left_type, right_type, result_type, eval,
                                  gen);
    }
  };

private:
  Table table;

public:
  BinopTable() noexcept = default;

  auto lookup(Token op) noexcept -> std::optional<Binop> {
    auto found = table.find(op);
    if (found != table.end()) {
      return found;
    }
    return std::nullopt;
  }

  auto emplace(Token op) -> Binop {
    auto found = table.find(op);
    if (found != table.end()) {
      return found;
    }

    return table.emplace(std::make_pair(op, BinopOverloads{})).first;
  }
};

void InitializeBuiltinBinops(Environment *env);

} // namespace mint
