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
#include "adt/SourceLocation.hpp"
#include "type/Type.hpp"

#include "utility/Assert.hpp"

namespace mint::ir::detail {
class IrBase {
  type::Ptr m_cached_type;
  SourceLocation *m_source_location;

protected:
  void setSL(SourceLocation *sl) noexcept {
    MINT_ASSERT(sl != nullptr);
    m_source_location = sl;
  }

public:
  IrBase(SourceLocation *source_location) noexcept
      : m_cached_type(nullptr), m_source_location(source_location) {
    MINT_ASSERT(source_location != nullptr);
  }

  [[nodiscard]] SourceLocation *sourceLocation() noexcept {
    return m_source_location;
  }
  [[nodiscard]] SourceLocation *sourceLocation() const noexcept {
    return m_source_location;
  }

  [[nodiscard]] type::Ptr cachedType() const noexcept { return m_cached_type; }
  type::Ptr cachedType(type::Ptr type) noexcept { return m_cached_type = type; }
};
} // namespace mint::ir::detail
