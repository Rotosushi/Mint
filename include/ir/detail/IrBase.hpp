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

#include "utility/Assert.hpp"

namespace mint::ir::detail {
class IrBase {
  SourceLocation const *m_source_location;

protected:
  void setSL(SourceLocation const *sl) noexcept {
    MINT_ASSERT(sl != nullptr);
    m_source_location = sl;
  }

public:
  IrBase(SourceLocation const *source_location) noexcept
      : m_source_location(source_location) {
    MINT_ASSERT(source_location != nullptr);
  }

  SourceLocation const &sourceLocation() const noexcept {
    return *m_source_location;
  }
};
} // namespace mint::ir::detail
