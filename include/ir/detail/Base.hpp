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

#include "scan/Location.hpp"
#include "type/Type.hpp"

#include "utility/Assert.hpp"

namespace mint {
namespace ir {
namespace detail {
// the base class of ir elements which need to keep
// track of a computed type and a source location.
class Base {
  mutable type::Ptr m_cached_type;
  Location *m_source_location;

public:
  Base(Location *source_location) noexcept
      : m_cached_type(nullptr), m_source_location(source_location) {
    MINT_ASSERT(source_location != nullptr);
  }

  auto cachedType(type::Ptr type) const noexcept -> type::Ptr {
    return m_cached_type = type;
  }

  auto hasCachedType() const noexcept -> bool {
    return m_cached_type == nullptr;
  }

  auto sourceLocation() const noexcept -> Location * {
    return m_source_location;
  }
};
} // namespace detail
} // namespace ir
} // namespace mint
