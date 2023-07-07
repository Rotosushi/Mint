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
#include <map>

#include "adt/Identifier.hpp"
// #include "adt/VectorMap.hpp"
#include "ast/Ast.hpp"

namespace mint {
class UseBeforeDefMap {
public:
  using Key = Identifier;
  using Value = std::pair<Identifier, ast::Ptr>;
  using Pair = std::pair<Key, Value>;
  using Map = std::multimap<Key, Value>;

  class Entry : public Map::iterator {
  public:
    Entry(Map::iterator iter) noexcept : Map::iterator(iter) {}

    auto undef() noexcept -> Identifier { return (*this)->first; }
    auto definition() noexcept -> Identifier { return (*this)->second.first; }
    auto ast() noexcept -> ast::Ptr & { return (*this)->second.second; }
  };

  class Range {
    std::pair<Map::iterator, Map::iterator> range;

  public:
    Range(std::pair<Map::iterator, Map::iterator> range) noexcept
        : range(range) {}

    auto pair() noexcept -> std::pair<Map::iterator, Map::iterator> & {
      return range;
    }
    auto begin() noexcept -> Entry { return range.first; }
    auto end() noexcept -> Entry { return range.second; }
  };

private:
  Map map;

public:
  [[nodiscard]] auto lookup(Identifier undef) noexcept -> Range {
    return map.equal_range(undef);
  }

  void reinsert(Identifier undef, Entry entry) noexcept {
    MINT_ASSERT(map.contains(entry.undef()));
    insert(undef, entry.definition(), entry.ast());
    erase(entry);
  }

  void erase(Entry entry) noexcept { map.erase(entry); }
  void erase(Range range) noexcept { map.erase(range.begin(), range.end()); }

  void insert(Identifier undef, Identifier definition, ast::Ptr ast) noexcept {
    map.emplace(undef, std::make_pair(definition, ast));
  }
};
} // namespace mint
