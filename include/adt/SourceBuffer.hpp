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
#include <fstream>
#include <list>
#include <string>

#include "adt/InputStream.hpp"
#include "adt/SourceLocation.hpp"

namespace mint {
// An abstract class representing a buffer of source code
class SourceBuffer {
public:
  using iterator = std::string::iterator;

private:
  // dependencies
  InputStream m_in;

  // details
  mutable std::list<SourceLocation> m_locations;
  Location m_current_location;
  std::string m_buffer;
  iterator m_cursor;
  iterator m_token;
  iterator m_marker;
  iterator m_end;

  void append(std::string_view text) noexcept;

public:
  SourceBuffer(InputStream &&in) noexcept : m_in(std::move(in)) {
    m_cursor = m_token = m_marker = m_end = m_buffer.end();
  }

  bool eof() const noexcept { return m_in.eof(); }
  bool good() const noexcept { return m_in.good(); }

  iterator &cursor() noexcept { return m_cursor; }
  iterator &token() noexcept { return m_token; }
  iterator &marker() noexcept { return m_marker; }
  iterator &end() noexcept { return m_end; }

  void prime() noexcept { m_token = m_cursor; }
  char &peek() noexcept { return *m_cursor; }
  void skip() noexcept { ++m_cursor; }
  void backup() noexcept { m_marker = m_cursor; }
  void restore() noexcept { m_cursor = m_token; }
  bool endOfInput() const noexcept { return m_cursor >= m_end; }

  std::string_view viewToken() const noexcept { return {m_token, m_cursor}; }

  void updateCurrentLocation() noexcept;
  Location const &currentLocation() const noexcept {
    return m_current_location;
  }

  std::string_view source(Location const &location) const noexcept;

  SourceLocation *getSourceLocation(Location const &location) const noexcept;

  void fill() noexcept {
    auto line = m_in.getline();
    append(line);
  }
};
} // namespace mint
