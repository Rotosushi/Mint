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
#include <variant>

#include "utility/Assert.hpp"

namespace mint {
// Abstracts away the difference between reading input
// from cin, and from a file. (mostly the part where
// cin is a static global, and files need to remain
// alive until their contents are fully buffered.)
class InputStream {
private:
  using Variant = std::variant<std::istream *, std::fstream>;

  Variant m_variant;

public:
  InputStream(std::istream &in) noexcept
      : m_variant(std::in_place_type<std::istream *>, &in) {}
  InputStream(std::fstream &&fin) noexcept
      : m_variant(std::in_place_type<std::fstream>, std::move(fin)) {}

  bool eof() const noexcept {
    if (std::holds_alternative<std::istream *>(m_variant)) {
      return std::get<std::istream *>(m_variant)->eof();
    } else {
      return std::get<std::fstream>(m_variant).eof();
    }
  }

  bool good() const noexcept {
    if (std::holds_alternative<std::istream *>(m_variant)) {
      return std::get<std::istream *>(m_variant)->good();
    } else {
      return std::get<std::fstream>(m_variant).good();
    }
  }

  std::string getline() noexcept {
    std::string buffer;

    if (std::holds_alternative<std::istream *>(m_variant)) {
      std::getline(*(std::get<std::istream *>(m_variant)), buffer, '\n');
    } else {
      std::getline(std::get<std::fstream>(m_variant), buffer, '\n');
    }

    buffer += '\n';
    return buffer;
  }
};
} // namespace mint
