// Copyright (C) 2024 Cade Weinberg
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
#include <mutex>
#include <optional>
#include <queue>

namespace mint {
template <class T> class SyncQueue {
private:
  std::mutex m_mutex;
  std::queue<T> m_queue;

public:
  std::optional<bool> try_empty() {
    std::unique_lock lock(m_mutex, std::defer_lock);
    if (!lock.try_lock()) {
      return std::nullopt;
    }

    return m_queue.empty();
  }

  bool try_push(T t) {
    std::unique_lock lock(m_mutex, std::defer_lock);
    if (!lock.try_lock()) {
      return false;
    }

    m_queue.emplace(std::move(t));
    return true;
  }

  std::optional<T> try_pop() {
    std::unique_lock lock(m_mutex, std::defer_lock);

    if (!lock.try_lock()) {
      return std::nullopt;
    }

    if (m_queue.empty()) {
      return std::nullopt;
    }

    std::optional<T> result{std::move(m_queue.back())};
    m_queue.pop();
    return result;
  }
};
} // namespace mint
