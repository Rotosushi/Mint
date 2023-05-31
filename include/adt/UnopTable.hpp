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

#include "error/Error.hpp"

namespace mint {
class Environment;

using UnopEvalFn = Result<Ast::Pointer> (*)(Ast *right, Environment *env);

struct UnopOverload {
  Type::Pointer right_type;
  Type::Pointer result_type;
  UnopEvalFn eval;

  [[nodiscard]] auto operator()(Ast *right, Environment *env) {
    return eval(right, env);
  }
};

class UnopOverloads {
  std::vector<UnopOverload> overloads;

public:
  auto lookup(Type::Pointer right_type) noexcept
      -> std::optional<UnopOverload> {
    for (auto &overload : overloads) {
      if (right_type == overload.right_type) {
        return overload;
      }
    }
    return std::nullopt;
  }

  auto emplace(Type::Pointer right_type, Type::Pointer result_type,
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
  using Table = std::unordered_map<Key, Value>;

  class Unop {
    Table::iterator iter;

  public:
    Unop(Table::iterator iter) noexcept : iter(iter) {}

    auto lookup(Type::Pointer right_type) noexcept
        -> std::optional<UnopOverload> {
      return iter->second.lookup(right_type);
    }

    auto emplace(Type::Pointer right_type, Type::Pointer result_type,
                 UnopEvalFn eval) noexcept -> UnopOverload {
      return iter->second.emplace(right_type, result_type, eval);
    }
  };

private:
  Table table;

public:
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

    return table.emplace(op, UnopOverloads{}).first;
  }
};

void InitializeBuiltinUnops(Environment *env);
} // namespace mint
