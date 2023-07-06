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
#include <vector>

#include "utility/Allocator.hpp"

namespace mint {
template <class Key> class VectorSet {
public:
  using Set = std::vector<Key, PolyAllocator<Key>>;
  using iterator = typename Set::iterator;

private:
  Set m_set;

public:
  VectorSet(Allocator &allocator) noexcept : m_set(allocator) {}

  [[nodiscard]] auto begin() noexcept { return m_set.begin(); }
  [[nodiscard]] auto end() noexcept { return m_set.end(); }

  [[nodiscard]] auto find(Key key) noexcept -> iterator {
    auto cursor = m_set.begin();
    auto end = m_set.end();

    while (cursor != end) {
      if (*cursor == key)
        return cursor;

      ++cursor;
    }
    return end;
  }

  [[nodiscard]] auto try_emplace(Key key) noexcept
      -> std::pair<iterator, bool> {
    auto found = find(key);
    if (found != end())
      return {found, false};

    m_set.emplace_back(key);
    return {std::prev(end()), true};
  }
};
} // namespace mint
