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
#include "adt/Recoverable.hpp"
#include "utility/Abort.hpp"
#include "utility/Assert.hpp"

namespace mint {
// Represents the Result of a given algorithm, which may
// succeed, fail, or fail in a recoverable way.
template <class T> class Result {
  using Variant =
      // #NOTE: I am torn between putting Recoverable, Recovered
      // here in this variant, or in the Error class itself.
      // its here because it is simple conceptually to ask
      // "is this result recovered"
      // however its also simple to ask "is the error recovered"
      // it just might not make sense to have a recovered error.
      std::variant<std::monostate, T, Recoverable, Recovered, Error>;

  Variant data;

public:
  Result() noexcept = default;
  Result(T t) noexcept : data(std::in_place_type<T>, std::move(t)) {}
  Result(Recoverable r) noexcept
      : data(std::in_place_type<Recoverable>, std::move(r)) {}
  Result(Recovered r) noexcept
      : data(std::in_place_type<Recovered>, std::move(r)) {}
  Result(Error e) noexcept : data(std::in_place_type<Error>, std::move(e)) {}
  Result(Error::Kind kind) noexcept : data(std::in_place_type<Error>, kind) {}
  Result(Error::Kind kind, Location location, std::string_view message) noexcept
      : data(std::in_place_type<Error>, kind, location, message) {}

  operator bool() const noexcept { return success(); }

  [[nodiscard]] auto success() const noexcept -> bool {
    return std::holds_alternative<T>(data);
  }

  [[nodiscard]] auto recoverable() const noexcept -> bool {
    return std::holds_alternative<Recoverable>(data);
  }

  [[nodiscard]] auto recovered() const noexcept -> bool {
    return std::holds_alternative<Recovered>(data);
  }

  [[nodiscard]] auto failure() const noexcept -> bool {
    return std::holds_alternative<Error>(data);
  }

  [[nodiscard]] auto value() noexcept -> T & {
    MINT_ASSERT(success());
    return std::get<T>(data);
  }
  [[nodiscard]] auto unknown() noexcept -> Recoverable & {
    MINT_ASSERT(recoverable());
    return std::get<Recoverable>(data);
  }
  [[nodiscard]] auto error() noexcept -> Error {
    if (failure())
      return std::get<Error>(data);

    if (recoverable())
      return std::get<Recoverable>(data).toError();

    abort("cannot create an error from the given result.");
  }
};
} // namespace mint
