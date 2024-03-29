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
#include <cstdint>

namespace mint {
struct Location {
  std::size_t fline;
  std::size_t fcolumn;
  std::size_t lline;
  std::size_t lcolumn;

  ~Location() noexcept = default;
  Location() noexcept : fline(1), fcolumn(0), lline(1), lcolumn(0) {}
  Location(const Location &other) noexcept = default;
  Location(Location &&other) noexcept = default;
  Location(std::size_t fl, std::size_t fc, std::size_t ll,
           std::size_t lc) noexcept
      : fline(fl), fcolumn(fc), lline(ll), lcolumn(lc) {}
  Location(Location &lhs, Location &rhs) noexcept
      : fline(lhs.fline), fcolumn(lhs.fcolumn), lline(rhs.fline),
        lcolumn(rhs.lcolumn) {}

  auto operator=(const Location &other) noexcept -> Location & = default;
  auto operator=(Location &&other) noexcept -> Location & = default;
};
} // namespace mint
