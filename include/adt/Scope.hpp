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

#include "ast/Ast.hpp"

namespace mint {
class Scope {
public:
  using Key = Identifier;
  using Value = std::pair<Type::Pointer, Ast::Value *>;
  using Table = std::unordered_map<Key, Value>;
  using iterator = typename Table::iterator;

  class Binding {
  private:
    iterator binding;

  public:
    Binding(iterator binding) noexcept : binding(binding) {}

    [[nodiscard]] auto name() const noexcept -> const Key & {
      return binding->first;
    }
    [[nodiscard]] auto type() const noexcept -> Type::Pointer {
      return binding->second.first;
    }
    [[nodiscard]] auto value() const noexcept -> Ast::Value * {
      return binding->second.second;
    }
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

  auto bind(Key key, Value value) noexcept -> Binding {
    auto pair = table.try_emplace(key, value);
    return pair.first;
  }

  auto lookup(Key key) const noexcept -> std::optional<Binding> {
    auto found = table.find(key);
    if (found == table.end()) {
      return std::nullopt;
    }
    return found;
  }
}
} // namespace mint
