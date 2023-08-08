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

#include "utility/Assert.hpp"

namespace mint {
// an array whose length can be specified at runtime
template <class T> class Array {
public:
  using iterator = T *;
  using const_iterator = T const *;
  using reference = T &;
  using const_reference = T const &;

private:
  std::size_t m_capacity;
  std::size_t m_size;
  std::unique_ptr<T[]> m_array;

public:
  Array() noexcept : m_size(0), m_capacity(0), m_array() {}
  Array(std::size_t size) noexcept
      : m_size(0), m_capacity(size),
        m_array(std::make_unique_for_overwrite<T[]>(size)) {}
  ~Array() noexcept = default;
  Array(Array const &other) noexcept
      : m_size(other.m_size), m_capacity(other.m_capacity),
        m_array(std::make_unique_for_overwrite<T[]>(other.size())) {
    std::uninitialized_move_n((other.begin(), other.size(), &m_array[0]));
  }
  Array(Array &&other) noexcept = default;
  auto operator=(Array const &other) noexcept -> Array & {
    if (this == &other)
      return *this;

    m_size = other.m_size;
    m_capacity = m_size;
    m_array = std::make_unique_for_overwrite<T[]>(other.size());
    std::uninitialized_move_n(other.begin(), other.size(), &m_array[0]);
  }
  auto operator=(Array &&other) noexcept -> Array & = default;

  void resize(std::size_t size) noexcept {
    auto old_array = std::move(m_array);
    m_array = std::make_unique_for_overwrite<T[]>(size);

    if (size <= m_size) {
      std::uninitialized_move_n((&old_array[0]), size, (&m_array[0]));
    } else {
      std::uninitialized_move_n((&old_array[0]), m_size, (&m_array[0]));
    }
    m_capacity = size;
  }

  void swap(Array &other) noexcept {
    std::swap(m_size, other.m_size);
    std::swap(m_array, other.m_array);
  }

  [[nodiscard]] auto empty() const noexcept -> bool { return m_size == 0; }
  [[nodiscard]] auto size() const noexcept -> std::size_t { return m_size; }
  [[nodiscard]] auto capacity() const noexcept -> std::size_t {
    return m_capacity;
  }

  [[nodiscard]] auto begin() noexcept -> iterator { return &m_array[0]; }
  [[nodiscard]] auto end() noexcept -> iterator { return &m_array[m_size]; }
  [[nodiscard]] auto begin() const noexcept -> const_iterator {
    return &m_array[0];
  }
  [[nodiscard]] auto end() const noexcept -> const_iterator {
    return &m_array[m_size];
  }
  [[nodiscard]] auto cbegin() const noexcept -> const_iterator {
    return begin();
  }
  [[nodiscard]] auto cend() const noexcept -> const_iterator { return end(); }

  [[nodiscard]] auto operator[](std::size_t index) noexcept -> reference {
    MINT_ASSERT(index < m_size);
    return m_array[index];
  }

  [[nodiscard]] auto operator[](std::size_t index) const noexcept
      -> const_reference {
    MINT_ASSERT(index < m_size);
    return m_array[index];
  }

  template <class... Args>
  [[nodiscard]] reference emplace_back(Args &&...args) {
    if (m_size == m_capacity) {
      resize(m_capacity + 1 * 2);
    }

    T &element = &m_array[m_size++];
    element = T{std::forward<Args>(args)...};
    return element;
  }
};
} // namespace mint
