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
#include <memory> // std::shared_ptr

namespace mint {
namespace detail {
template <class T>
concept HasPtrMethod = requires(T a) {
  { a->ptr() } -> std::convertible_to<std::shared_ptr<T>>;
};
} // namespace detail

template <detail::HasPtrMethod T>
class UseCreateUnique : public std::enable_shared_from_this<T> {
protected:
  struct use_create_method {
    explicit use_create_method() = default;
  };

public:
  template <typename... Args> static auto create(Args &&...args) noexcept {
    return std::make_shared<T>(use_create_method{},
                               std::forward<Args>(args)...);
  }
};
} // namespace mint
