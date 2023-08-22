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
#include <istream>
#include <string>

#include "utility/Assert.hpp"

namespace mint {
// Abstracts away the difference between reading input
// from cin, and from a file. (mostly the part where
// cin is a static global, and files need to remain
// alive until their contents are fully buffered.)
class InputStream {
private:
  std::istream *m_in;

public:
  InputStream(std::istream *in) noexcept : m_in(in) {
    MINT_ASSERT(in != nullptr);
  }

  bool eof() const noexcept { return m_in->eof(); }
  bool good() const noexcept { return m_in->good(); }

  std::string getline() noexcept {
    std::string buffer;
    std::getline(*m_in, buffer, '\n');
    buffer += '\n';
    return buffer;
  }
};

class FileInputStream : InputStream {
private:
  std::fstream m_fin;

public:
  FileInputStream(std::fstream &&fin)
      : InputStream(&fin), m_fin(std::move(fin)) {}
};
} // namespace mint
