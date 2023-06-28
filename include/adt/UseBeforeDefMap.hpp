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
#include <unordered_map>

#include "adt/Identifier.hpp"
#include "ast/Ast.hpp"
#include "utility/Allocator.hpp"

namespace mint {
class UseBeforeDefMap {
public:
  using Key = Identifier;
  using Value = Ast::Ptr;
  using Pair = std::pair<const Key, Value>;
  using Map = std::unordered_multimap<Key, Value, std::hash<Key>,
                                      std::equal_to<Key>, PolyAllocator<Pair>>;
  using Range = std::pair<Map::iterator, Map::iterator>;

private:
  Map map;

public:
  UseBeforeDefMap(Allocator &allocator) noexcept
      : map(PolyAllocator<Pair>(allocator)) {}

  [[nodiscard]] auto lookup(Identifier name) noexcept -> Range {
    return map.equal_range(name);
  }

  auto insert(Identifier name, Ast::Ptr ast) noexcept -> Ast::Ptr {
    auto iter = map.insert(Pair{name, ast});
    return iter->second;
  }
};
} // namespace mint
