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

#include <list>
#include <utility>

namespace mint {
template <class Key, class Value> class Map {
public:
  using Pair = std::pair<Key, Value>;
  using Table = std::list<Pair>;
  using iterator = typename Table::iterator;

private:
  Table m_map;

public:
  [[nodiscard]] auto empty() const noexcept { return m_map.empty(); }

  [[nodiscard]] auto begin() noexcept { return m_map.begin(); }
  [[nodiscard]] auto end() noexcept { return m_map.end(); }

  [[nodiscard]] auto find(Key key) noexcept -> iterator {
    if (empty())
      return end();

    auto cursor = m_map.begin();
    auto end = m_map.end();
    while (cursor != end) {
      if (cursor->first == key)
        break;

      ++cursor;
    }
    return cursor;
  }

  template <class... Args>
  [[nodiscard]] auto try_emplace(Key key, Args &&...args) noexcept
      -> std::pair<iterator, bool> {
    auto found = find(key);
    if (found != end())
      return {found, false};

    m_map.emplace_back(key, std::forward<Args>(args)...);
    return {std::prev(end()), true};
  }

  void erase(Key key) noexcept {
    auto found = find(key);
    if (found != end())
      m_map.erase(found);
  }
  void erase(iterator iter) noexcept {
    if (iter != end())
      m_map.erase(iter);
  };
};

template <class Key, class Value> class Multimap {
public:
  using Pair = std::pair<Key, Value>;
  using Table = std::list<Pair>;
  using iterator = typename Table::iterator;
  using Range = std::pair<iterator, iterator>;

private:
  Table m_map;

public:
  [[nodiscard]] auto empty() const noexcept { return m_map.empty(); }

  [[nodiscard]] auto begin() noexcept { return m_map.begin(); }
  [[nodiscard]] auto end() noexcept { return m_map.end(); }

  [[nodiscard]] auto find(Key key) noexcept -> iterator {
    if (empty())
      return end();

    auto cursor = m_map.begin();
    auto end = m_map.end();
    while (cursor != end) {
      if (cursor->first == key)
        break;

      ++cursor;
    }

    return cursor;
  }

  [[nodiscard]] auto equal_range(Key key) noexcept -> Range {
    if (empty())
      return std::make_pair(end(), end());

    auto cursor = m_map.begin();
    auto end = m_map.end();
    while (cursor != end) {
      if (cursor->first == key) {
        auto range_end = cursor;
        do {
          ++range_end;
        } while (range_end->first == key);
        return {cursor, range_end};
      }

      ++cursor;
    }
    return {end, end};
  }

  auto emplace(Key key, Value value) noexcept -> iterator {
    auto range = equal_range(key);
    return m_map.emplace(range.second, key, value);
  }

  void erase(iterator iter) noexcept {
    if (iter != end())
      m_map.erase(iter);
  }
  void erase(iterator begin, iterator end) noexcept {
    if (begin != this->end())
      m_map.erase(begin, end);
  }
  void erase(Range range) noexcept {
    if (range.first != end())
      m_map.erase(range.first, range.second);
  }

  [[nodiscard]] auto contains(Key key) noexcept -> bool {
    auto found = find(key);
    return found != end();
  }
};
} // namespace mint
