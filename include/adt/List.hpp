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
#include <memory>

namespace mint {
template <class T> class List {
private:
  struct Node {
    T m_data;
    Node *m_prev;
    std::unique_ptr<Node> m_next;

    template <class... Args>
    Node(Node *prev, std::unique_ptr<Node> next, Args &&...args) noexcept
        : m_data(std::forward<Args>(args)...), m_prev(prev),
          m_next(std::move(next)) {}
    Node(Node *prev, std::unique_ptr<Node> next, T data) noexcept
        : m_data(std::move(data)), m_prev(prev), m_next(std::move(next)) {}
  };

public:
  class iterator {
  public:
    using reference = T &;
    using pointer = T *;

  private:
    Node *m_iter;

  public:
    iterator(Node *iter) noexcept : m_iter(iter) {}
    iterator(iterator const &other) noexcept : m_iter(other.m_iter) {}
    iterator(iterator &&other) noexcept : m_iter(other.m_iter) {
      other.m_iter = nullptr; // 'move' from the other iterator
    }
    auto operator=(iterator const &other) noexcept -> iterator & {
      if (this == &other)
        return *this;

      m_iter = other.m_iter;
      return *this;
    }
    auto operator=(iterator &&other) noexcept -> iterator & {
      if (this == &other)
        return *this;

      m_iter = other.m_iter;
      other.m_iter = nullptr;
      return *this;
    }

    auto operator==(iterator const &other) const noexcept -> bool {
      return m_iter == other.m_iter;
    }
    auto operator!=(iterator const &other) const noexcept -> bool {
      return !(*this == other);
    }

    auto operator*() const noexcept -> reference { return m_iter->m_data; }
    auto operator*() noexcept -> reference { return m_iter->m_data; }
    auto operator->() const noexcept -> pointer { return &(m_iter->m_data); }
    auto operator->() noexcept -> pointer { return &(m_iter->m_data); }

    auto operator++() noexcept -> iterator & {
      m_iter = m_iter->m_next.get();
      return *this;
    }
    auto operator++(int) noexcept -> iterator {
      iterator temp = *this;
      ++(*this);
      return temp;
    }
    auto operator--() noexcept -> iterator & {
      m_iter = m_iter->m_prev;
      return *this;
    }
    auto operator--(int) noexcept -> iterator {
      iterator temp = *this;
      --(*this);
      return temp;
    }
  };

private:
  std::size_t m_size;
  std::unique_ptr<Node> m_head;
  Node *m_last;

public:
  List() noexcept : m_size(0) {}
  List(List const &other) noexcept = delete;
  List(List &&other) noexcept
      : m_size(other.m_size), m_head(std::move(other.m_head)) {
    other.m_size = 0;
    other.m_head = nullptr;
  }
  auto operator=(List const &other) noexcept -> List & = delete;
  auto operator=(List &&other) noexcept -> List & {
    if (this == &other)
      return *this;

    m_size = other.m_size;
    m_head = std::move(other.m_head);
    return *this;
  }

  auto begin() noexcept -> iterator { return m_head.get(); }
  auto end() noexcept -> iterator { return nullptr; }

  auto empty() const noexcept -> bool { return m_head.get() == nullptr; }

  auto push_front(T data) noexcept -> iterator {
    auto *old_head = m_head.get();
    m_head = std::make_unique<Node>(old_head, std::move(m_head), data);
    ++m_size;
    return m_head.get();
  }

  template <class... Args>
  auto emplace_front(Args &&...args) noexcept -> iterator {
    auto *old_head = m_head.get();
    m_head = std::make_unique<Node>(old_head, std::move(m_head),
                                    std::forward<Args>(args)...);
    ++m_size;
    return m_head.get();
  }

  auto push_back(T data) noexcept -> iterator {
    m_last->m_next = std::make_unique<Node>(m_last, nullptr, data);
    m_last = m_last->m_next.get();
    ++m_size;
    return m_last;
  }

  template <class... Args>
  auto emplace_back(Args &&...args) noexcept -> iterator {
    m_last->m_next =
        std::make_unique<Node>(m_last, nullptr, std::forward<Args>(args)...);
    m_last = m_last->m_next.get();
    ++m_size;
    return m_last;
  }

  auto insert(iterator iter, T data) noexcept -> iterator {
    Node *prev = iter.m_iter->m_prev;
    prev->m_next = std::make_unique<Node>(prev, std::move(prev->m_next), data);
    iter->m_prev = node.get();
    ++m_size;
    return prev->m_next.get();
  }

  template <class... Args>
  auto emplace(iterator iter, Args &&...args) noexcept -> iterator {
    Node *prev = iter.m_iter->m_prev;
    prev->m_next = std::make_unique<Node>(prev, std::move(prev->m_next),
                                          std::forward<Args>(args)...);
    iter->m_prev = node.get();
    ++m_size;
    return prev->m_next.get();
  }

  void erase(iterator iter) noexcept {
    Node *prev = iter.m_iter->m_prev;
    prev->m_next = std::move(iter.m_iter->m_next);
    --m_size;
    return prev->m_next.get();
  }

  void erase(iterator begin, iterator end) noexcept {
    if ((begin == iterator{nullptr}) || (begin == end))
      return;

    while (begin != end) {
      erase(begin);
    }
  }
};
} // namespace mint
