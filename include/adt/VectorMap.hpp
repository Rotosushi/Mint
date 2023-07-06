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
#include <utility>
#include <vector>

namespace mint {
template <class Key, class Value> class VectorMap {
public:
  using Pair = std::pair<Key, Value>;
  using Map = std::vector<Pair>;
  using iterator = typename Map::iterator;

private:
  Map m_map;

public:
  [[nodiscard]] auto empty() const noexcept { return m_map.empty(); }

  [[nodiscard]] auto begin() noexcept { return m_map.begin(); }
  [[nodiscard]] auto end() noexcept { return m_map.end(); }

  [[nodiscard]] auto find(Key key) noexcept -> iterator {
    auto cursor = m_map.begin();
    auto end = m_map.end();
    while (cursor != end) {
      if (cursor->first == key)
        return cursor;

      ++cursor;
    }
    return end;
  }

  [[nodiscard]] auto try_emplace(Key key, Value value) noexcept
      -> std::pair<iterator, bool> {
    auto found = find(key);
    if (found != end())
      return {found, false};

    m_map.emplace_back(key, value);
    return {std::prev(end()), true};
  }

  auto erase(Key key) noexcept {
    auto found = find(key);
    if (found != end())
      return erase(found);
    else
      return end();
  }
  auto erase(iterator iter) noexcept { return m_map.erase(iter); };
};
} // namespace mint
