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
#include <memory_resource> // std::pmr::memory_resource

#include "utility/Assert.hpp"

namespace mint {
template <class T> class ManagedPointer {
  using Type = std::remove_cvref_t<T>;
  using Resource = std::pmr::memory_resource;

  Resource *resource;
  Type *pointer;

public:
  ManagedPointer(Resource *resource) noexcept
      : resource(resource), pointer(nullptr) {
    MINT_ASSERT(resource != nullptr);
  }
  ManagedPointer(Type *pointer, Resource *resource) noexcept
      : resource(resource), pointer(pointer) {
    MINT_ASSERT(pointer != nullptr);
    MINT_ASSERT(resource != nullptr);
  }
  ~ManagedPointer() noexcept {
    if (pointer != nullptr)
      resource->deallocate(pointer, sizeof(T));
  }

  ManagedPointer(ManagedPointer &&other) noexcept {
    if (this == &other)
      return;

    resource = other.resource;
    pointer = other.pointer;
    other.pointer = nullptr;
  }

  auto operator=(ManagedPointer &&other) noexcept -> ManagedPointer & {
    if (this == &other)
      return *this;

    resource = other.resource;
    pointer = other.pointer;
    other.pointer = nullptr;
    return *this;
  }

  ManagedPointer(const ManagedPointer &other) noexcept = delete;
  auto operator=(const ManagedPointer &other) noexcept = delete;

  [[nodiscard]] auto get() noexcept { return pointer; }
  [[nodiscard]] auto getResource() noexcept { return resource; }
};
} // namespace mint
