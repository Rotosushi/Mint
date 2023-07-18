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
#include <expected>

#include "error/Error.hpp"
#include "utility/Assert.hpp"

namespace mint {
template <class T> class Result {
  std::expected<T, Error> data;

public:
  Result(T t) noexcept : data(std::move(t)) {}
  Result(Error e) noexcept : data(std::unexpect, std::move(e)) {}
  Result(Error::Kind kind) noexcept : data(std::unexpect, kind) {}
  Result(Error::Kind kind, Location location, std::string_view message) noexcept
      : data(std::unexpect, kind, location, message) {}

  operator bool() const noexcept { return data.has_value(); }

  [[nodiscard]] auto hasValue() const noexcept -> bool {
    return data.has_value();
  }

  [[nodiscard]] auto hasError() const noexcept -> bool {
    return !data.has_value();
  }

  [[nodiscard]] auto value() noexcept -> T & {
    MINT_ASSERT(hasValue());
    return data.value();
  }
  [[nodiscard]] auto error() noexcept -> Error & {
    MINT_ASSERT(hasError());
    return data.error();
  }
};
} // namespace mint
