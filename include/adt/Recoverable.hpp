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
#include <variant>

#include "adt/Error.hpp"
#include "adt/Identifier.hpp"
#include "utility/Assert.hpp"

namespace mint {
class Scope;

// Represents the case where the algorithm failed in a
// recoverable way, and that was handled by the algorithm.
// so the driver code can continue processing.
class Recovered {};

// Represents the case where an algorithm can go no further
// yet may be recoverable.
class Recoverable {
public:
  // #TODO: hold the data from the use before definition. and
  // return this in the result when the use before def happens.
  // and catch this error wile typechecking the definition and add
  // it too the use before def in the map.
  struct UBD {
    Identifier undef_name;
    std::shared_ptr<Scope> local_scope;
  };

  using Data = std::variant<std::monostate, UBD>;

private:
  Data m_data;

public:
  Recoverable() noexcept = default;
  Recoverable(Identifier undef, std::shared_ptr<Scope> local_scope) noexcept
      : m_data(std::in_place_type<UBD>, undef, std::move(local_scope)) {}
  ~Recoverable() noexcept = default;
  Recoverable(Recoverable const &other) noexcept = default;
  Recoverable(Recoverable &&other) noexcept = default;
  auto operator=(Recoverable const &other) noexcept -> Recoverable & = default;
  auto operator=(Recoverable &&other) noexcept -> Recoverable & = default;

  [[nodiscard]] constexpr auto data() noexcept -> Data & { return m_data; }

  [[nodiscard]] auto toError() noexcept -> Error;
};
} // namespace mint
