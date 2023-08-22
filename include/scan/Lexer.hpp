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
#include <utility>

#include "adt/SourceBuffer.hpp"
#include "scan/Token.hpp"

namespace mint {
class Lexer {
public:
  using iterator = SourceBuffer::iterator;

private:
  SourceBuffer *m_source;

public:
  Lexer(SourceBuffer *source) noexcept : m_source(source) {
    MINT_ASSERT(source != nullptr);
  }

  SourceBuffer *exchangeSource(SourceBuffer *source) noexcept {
    MINT_ASSERT(source != nullptr);
    return std::exchange(m_source, source);
  }

  iterator &cursor() noexcept { return m_source->cursor(); }
  iterator &token() noexcept { return m_source->token(); }
  iterator &marker() noexcept { return m_source->marker(); }
  iterator &end() noexcept { return m_source->end(); }

  void prime() noexcept { m_source->prime(); }
  char &peek() noexcept { return m_source->peek(); }
  void skip() noexcept { m_source->skip(); }
  void backup() noexcept { m_source->backup(); }
  void restore() noexcept { m_source->restore(); }
  bool endOfInput() noexcept { return m_source->endOfInput(); }

  bool eof() const noexcept { return m_source->eof(); }
  bool good() const noexcept { return m_source->good(); }

  void fill() noexcept { m_source->fill(); }

  std::string_view viewToken() const noexcept { return m_source->viewToken(); }

  Location const &currentLocation() const noexcept {
    return m_source->currentLocation();
  }

  void updateCurrentLocation() noexcept {
    return m_source->updateCurrentLocation();
  }

  SourceLocation source(Location const &location) const noexcept {
    return m_source->source(location);
  }

  Token lex() noexcept;
};
} // namespace mint
