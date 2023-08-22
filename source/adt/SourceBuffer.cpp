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
#include "adt/SourceBuffer.hpp"

namespace mint {
void SourceBuffer::updateCurrentLocation() noexcept {
  m_current_location.fline = m_current_location.lline;
  m_current_location.fcolumn = m_current_location.lcolumn;

  auto length = m_cursor - m_token;
  for (std::ptrdiff_t i = 0; i < length; ++i) {
    if (m_token[i] == '\n') {
      ++m_current_location.lline;
      m_current_location.lcolumn = 0;
      m_current_location.fcolumn = 0;
    } else {
      ++m_current_location.lcolumn;
    }
  }
}

void SourceBuffer::append(std::string_view text) noexcept {
  if (m_buffer.empty()) {
    m_buffer.append(text);
    m_end = m_buffer.end();
    m_marker = m_token = m_cursor = m_buffer.begin();
    return;
  }

  auto begin = m_buffer.begin();
  auto coffset = std::distance(begin, m_cursor);
  auto moffset = std::distance(begin, m_marker);
  auto toffset = std::distance(begin, m_token);

  m_buffer.append(text);
  begin = m_buffer.begin();
  m_end = m_buffer.end();

  m_cursor = begin + coffset;
  m_marker = begin + moffset;
  m_token = begin + toffset;
}

SourceLocation SourceBuffer::source(Location const &location) const noexcept {
  auto cursor = m_buffer.begin();
  auto end = m_buffer.end();
  std::size_t lines_seen = 1;

  while ((lines_seen < location.fline) && (cursor != end)) {
    if (*cursor == '\n')
      ++lines_seen;

    ++cursor;
  }

  if (cursor != end) {
    auto eol = cursor;

    while (eol != end && *eol != '\n')
      ++eol;

    return {saveLocation(location), {cursor, eol}};
  }

  return {saveLocation(location), {}};
}

} // namespace mint
