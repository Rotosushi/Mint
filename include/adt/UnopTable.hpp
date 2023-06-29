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
#include "utility/Allocator.hpp"

namespace mint {
class Environment;

using UnopEvalFn = Result<ast::Ptr> (*)(ast::Ptr &right, Environment &env);

struct UnopOverload {
  type::Ptr right_type;
  type::Ptr result_type;
  UnopEvalFn eval;

  [[nodiscard]] auto operator()(ast::Ptr &right, Environment &env) {
    return eval(right, env);
  }
};

class UnopOverloads {
  std::vector<UnopOverload, PolyAllocator<UnopOverload>> overloads;

public:
  UnopOverloads(Allocator &allocator) noexcept : overloads(allocator) {
    overloads.reserve(2);
  }

  auto lookup(type::Ptr right_type) noexcept -> std::optional<UnopOverload> {
    for (auto &overload : overloads) {
      if (right_type == overload.right_type) {
        return overload;
      }
    }
    return std::nullopt;
  }

  auto emplace(type::Ptr right_type, type::Ptr result_type,
               UnopEvalFn eval) noexcept -> UnopOverload {
    auto found = lookup(right_type);
    if (found) {
      return found.value();
    }

    return overloads.emplace_back(UnopOverload{right_type, result_type, eval});
  }
};

class UnopTable {
public:
  using Key = Token;
  using Value = UnopOverloads;
  using Pair = std::pair<const Key, Value>;
  using Table = std::unordered_map<Key, Value, std::hash<Key>,
                                   std::equal_to<Key>, PolyAllocator<Pair>>;

  class Unop {
    Table::iterator iter;

  public:
    Unop(Table::iterator iter) noexcept : iter(iter) {}

    auto lookup(type::Ptr right_type) noexcept -> std::optional<UnopOverload> {
      return iter->second.lookup(right_type);
    }

    auto emplace(type::Ptr right_type, type::Ptr result_type,
                 UnopEvalFn eval) noexcept -> UnopOverload {
      return iter->second.emplace(right_type, result_type, eval);
    }
  };

private:
  Allocator *allocator;
  Table table;

public:
  UnopTable(Allocator &allocator) noexcept
      : allocator(&allocator), table(PolyAllocator(allocator)) {}

  auto lookup(Token op) noexcept -> std::optional<Unop> {
    auto found = table.find(op);
    if (found != table.end()) {
      return found;
    }
    return std::nullopt;
  }

  auto emplace(Token op) noexcept -> Unop {
    auto found = table.find(op);
    if (found != table.end()) {
      return found;
    }

    return table.emplace(op, UnopOverloads{*allocator}).first;
  }
};

void InitializeBuiltinUnops(Environment *env);
} // namespace mint
