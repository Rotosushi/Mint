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
#include <ostream>         // std::ostream
#include <source_location> // std::source_location
#include <string_view>     // std::string_view

namespace mint {
inline void
log(std::ostream &out, std::string_view message,
    std::source_location location = std::source_location::current()) {
  out << "file: " << location.file_name() << "(" << location.line() << ":"
      << location.column() << ")"
      << " `" << location.function_name() << "`: " << message << "\n";
}
} // namespace mint
