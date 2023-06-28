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
#include "utility/Allocator.hpp"

namespace mint {
class Environment;
/*

*/
using BinopEvalFn = Result<Ast::Ptr> (*)(Ast *left, Ast *right,
                                         Environment *env);

struct BinopOverload {
  Type::Ptr left_type;
  Type::Ptr right_type;
  Type::Ptr result_type;
  BinopEvalFn eval;

  [[nodiscard]] auto operator()(Ast *left, Ast *right, Environment *env) {
    return eval(left, right, env);
  }
};

class BinopOverloads {
  std::vector<BinopOverload, PolyAllocator<BinopOverload>> overloads;

public:
  BinopOverloads(Allocator &allocator) noexcept : overloads(allocator) {
    overloads.reserve(2);
  }

  auto lookup(Type::Ptr left_type, Type::Ptr right_type) noexcept
      -> std::optional<BinopOverload> {
    for (auto &overload : overloads) {
      if (left_type == overload.left_type &&
          right_type == overload.right_type) {
        return overload;
      }
    }
    return std::nullopt;
  }

  auto emplace(Type::Ptr left_type, Type::Ptr right_type, Type::Ptr result_type,
               BinopEvalFn eval) noexcept -> BinopOverload {
    auto found = lookup(left_type, right_type);
    if (found) {
      return found.value();
    }

    return overloads.emplace_back(
        BinopOverload{left_type, right_type, result_type, eval});
  }
};

class BinopTable {
public:
  using Key = Token;
  using Value = BinopOverloads;
  using Pair = std::pair<const Key, Value>;
  using Table = std::unordered_map<Key, Value, std::hash<Key>,
                                   std::equal_to<Key>, PolyAllocator<Pair>>;

  class Binop {
    Table::iterator iter;

  public:
    Binop(Table::iterator iter) noexcept : iter(iter) {}

    auto lookup(Type::Ptr left_type, Type::Ptr right_type) noexcept
        -> std::optional<BinopOverload> {
      return iter->second.lookup(left_type, right_type);
    }

    auto emplace(Type::Ptr left_type, Type::Ptr right_type,
                 Type::Ptr result_type, BinopEvalFn eval) noexcept
        -> BinopOverload {
      return iter->second.emplace(left_type, right_type, result_type, eval);
    }
  };

private:
  Allocator *allocator;
  Table table;

public:
  BinopTable(Allocator &allocator) noexcept
      : allocator(&allocator), table(PolyAllocator<Pair>(allocator)) {}

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

    return table.emplace(std::make_pair(op, BinopOverloads{*allocator})).first;
  }
};

void InitializeBuiltinBinops(Environment *env);

} // namespace mint
