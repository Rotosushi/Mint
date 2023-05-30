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
#include "utility/ManagedPointer.hpp"

namespace mint {
class Allocator {
public:
  using Resource = std::pmr::memory_resource;

private:
  Resource *resource;

public:
  Allocator(Resource *resource) noexcept : resource(resource) {
    MINT_ASSERT(resource != nullptr);
  }

  template <class T, class... Args>
  [[nodiscard]] auto construct(Args &&...args) noexcept -> ManagedPointer<T> {
    auto *alloc = resource->allocate(sizeof(T));
    T *t = std::construct_at<T>(static_cast<T *>(alloc),
                                std::forward<Args>(args)...);
    return {t, resource};
  }
};
} // namespace mint
