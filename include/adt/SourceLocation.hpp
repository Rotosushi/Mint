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
#include <string_view>

#include "scan/Location.hpp"
#include "utility/Assert.hpp"

namespace mint {
class SourceLocation {
private:
  Location m_location;
  // #TODO: replace with a pointer to a fs::path or SourceFile
  std::string_view m_view;

public:
  SourceLocation(Location location, std::string_view view) noexcept
      : m_location(location), m_view(view) {}

  Location const &location() const noexcept { return m_location; }
  std::string_view const &view() const noexcept { return m_view; }
};
} // namespace mint
