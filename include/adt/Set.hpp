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
#include <list>

namespace mint {
template <class Key> class Set {
public:
  using Elements = std::list<Key>;
  using iterator = typename Elements::iterator;

private:
  Elements m_elements;

public:
  [[nodiscard]] auto begin() noexcept { return m_elements.begin(); }
  [[nodiscard]] auto end() noexcept { return m_elements.end(); }

  [[nodiscard]] auto find(Key key) noexcept -> iterator {
    auto cursor = m_elements.begin();
    auto end = m_elements.end();

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

    m_elements.emplace_back(key);
    return {std::prev(end()), true};
  }

  void erase(Key key) noexcept {
    auto found = find(key);

    if (found != end())
      m_elements.erase(found);
  }

  void erase(iterator element) noexcept { m_elements.erase(element); }
};
} // namespace mint
