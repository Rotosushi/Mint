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
#include <iterator>
#include <string>
#include <string_view>

#include "scan/Location.hpp"
#include "scan/Token.hpp"

namespace mint {
class Scanner {
private:
  using iterator = std::string::iterator;

  Location location;
  std::string buffer;
  iterator cursor;
  iterator token;
  iterator marker;
  iterator end;

  void UpdateLocation() noexcept;

public:
  Scanner() noexcept;
  Scanner(std::string_view input) noexcept;

  auto view() const noexcept -> std::string_view;
  void reset() noexcept;
  auto endOfInput() const noexcept -> bool;

  void append(std::string_view text) noexcept;
  auto getText() const noexcept -> std::string_view;
  auto getLocation() const noexcept -> Location;
  auto scan() noexcept -> Token;
};
} // namespace mint