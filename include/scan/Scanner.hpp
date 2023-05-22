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

  void UpdateLocation() noexcept {
    auto length = cursor - token;
    location.fline = location.lline;
    location.fcolumn = location.lcolumn;

    for (std::ptrdiff_t i = 0; i < length; ++i) {
      if (token[i] == '\n') {
        ++location.lline;
        location.lcolumn = 0;
        location.fcolumn = 0;
      } else {
        ++location.lcolumn;
      }
    }
  }

public:
  Scanner() noexcept : location{1, 0, 1, 0} {
    end = marker = token = cursor = buffer.end();
  }

  Scanner(std::string_view input) noexcept
      : location{1, 0, 1, 0}, buffer{input} {
    end = buffer.end();
    marker = token = cursor = buffer.begin();
  }

  auto view() const noexcept -> std::string_view {
    return {buffer.begin(), buffer.end()};
  }

  void reset() noexcept {
    location = {1, 0, 1, 0};
    buffer.clear();
    end = cursor = marker = token = buffer.end();
  }

  auto endOfInput() const noexcept -> bool { return cursor == end; }

  void append(std::string_view text) noexcept {
    auto begin = buffer.begin();
    auto cursor_offset = std::distance(begin, cursor);
    auto marker_offset = std::distance(begin, marker);
    auto token_offset = std::distance(begin, token);

    buffer.append(text);

    begin = buffer.begin();
    end = buffer.end();
    cursor = begin + cursor_offset;
    marker = begin + marker_offset;
    token = begin + token_offset;
  }

  auto getText() const noexcept -> std::string_view { return {token, cursor}; }
  auto getLocation() const noexcept { return location; }
  auto scan() noexcept -> Token;
};
} // namespace mint