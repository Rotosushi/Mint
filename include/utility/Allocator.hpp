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
#include <memory_resource>

namespace mint {
using Allocator = std::pmr::polymorphic_allocator<>;

template <class T> using PolyAllocator = std::pmr::polymorphic_allocator<T>;

template <class T> struct Deleter {
  Allocator *allocator;
  Deleter(Allocator &allocator) noexcept : allocator(&allocator) {}

  void operator()(T *p) {
    std::allocator_traits<Allocator>::destroy(*allocator, p);
    std::allocator_traits<Allocator>::deallocate(
        *allocator, reinterpret_cast<std::byte *>(p), sizeof(T));
  }
};

template <class T> using allocator_unique = std::unique_ptr<T, Deleter<T>>;

template <class T, class... Args>
inline auto allocateUniquePtr(Allocator &allocator, Args &&...args) {
  auto *alloc = reinterpret_cast<T *>(
      std::allocator_traits<Allocator>::allocate(allocator, sizeof(T)));
  std::allocator_traits<Allocator>::construct<T>(allocator, alloc,
                                                 std::forward<Args>(args)...);
  return allocator_unique(alloc, Deleter<T>{allocator});
}
} // namespace mint
