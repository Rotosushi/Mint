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
#include <variant>

#include "adt/Error.hpp"
#include "utility/Assert.hpp"

namespace mint {
class Unknown {};

template <class T> class Result {
  using Variant = std::variant<T, Unknown, Error>;

  Variant data;

public:
  Result() noexcept : data(std::in_place_type<Unknown>) {}
  Result(T t) noexcept : data(std::in_place_type<T>, std::move(t)) {}
  Result(Error e) noexcept : data(std::in_place_type<Error>, std::move(e)) {}
  Result(Error::Kind kind) noexcept : data(std::in_place_type<Error>, kind) {}
  Result(Error::Kind kind, Location location, std::string_view message) noexcept
      : data(std::in_place_type<Error>, kind, location, message) {}

  operator bool() const noexcept { return success(); }

  [[nodiscard]] auto success() const noexcept -> bool {
    return std::holds_alternative<T>(data);
  }

  [[nodiscard]] auto unknown() const noexcept -> bool {
    return std::holds_alternative<Unknown>(data);
  }

  [[nodiscard]] auto failure() const noexcept -> bool {
    return std::holds_alternative<Error>(data);
  }

  [[nodiscard]] auto value() noexcept -> T & {
    MINT_ASSERT(success());
    return std::get<T>(data);
  }
  [[nodiscard]] auto error() noexcept -> Error & {
    MINT_ASSERT(failure());
    return std::get<Error>(data);
  }
};
} // namespace mint
