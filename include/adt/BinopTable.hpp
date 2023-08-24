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

#include "adt/Result.hpp"
#include "ir/detail/Scalar.hpp"
#include "scan/Token.hpp"
#include "type/Type.hpp"

#include "llvm/IR/Value.h"

namespace mint {
class Environment;

using BinopEvalFn = ir::detail::Scalar (*)(ir::detail::Scalar left,
                                           ir::detail::Scalar right);
using BinopCodegenFn = llvm::Value *(*)(llvm::Value *left, llvm::Value *right,
                                        Environment &env);

struct BinopOverload {
  type::Ptr left_type;
  type::Ptr right_type;
  type::Ptr result_type;
  BinopEvalFn eval;
  BinopCodegenFn gen;

  [[nodiscard]] auto evaluate(ir::detail::Scalar left, ir::detail::Scalar right)
      -> ir::detail::Scalar;

  [[nodiscard]] auto codegen(llvm::Value *left, llvm::Value *right,
                             Environment &env) -> llvm::Value *;
};

class BinopOverloads {
  std::vector<BinopOverload> overloads;

public:
  BinopOverloads() noexcept { overloads.reserve(2); }

  auto lookup(type::Ptr left_type, type::Ptr right_type) noexcept
      -> std::optional<BinopOverload>;

  auto emplace(type::Ptr left_type, type::Ptr right_type, type::Ptr result_type,
               BinopEvalFn eval, BinopCodegenFn gen) noexcept -> BinopOverload;
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
        -> std::optional<BinopOverload>;

    auto emplace(type::Ptr left_type, type::Ptr right_type,
                 type::Ptr result_type, BinopEvalFn eval,
                 BinopCodegenFn gen) noexcept -> BinopOverload;
  };

private:
  Table table;

public:
  BinopTable() noexcept = default;

  auto lookup(Token op) noexcept -> std::optional<Binop>;

  auto emplace(Token op) -> Binop;
};

void InitializeBuiltinBinops(Environment *env);

} // namespace mint
