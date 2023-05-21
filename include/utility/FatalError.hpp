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
#include <iostream>

#include "utility/Assert.hpp"
#include "utility/Log.hpp"

namespace mint {
[[noreturn]] inline void
fatalError(std::string_view message,
           std::source_location location =
               std::source_location::current()) noexcept(true) {
  log(std::cerr, message, location);
  // if we are in a debug build, emit a breakpoint instruction here
  MINT_ASSERT(false && "fatalError");

  std::exit(EXIT_FAILURE);
}
} // namespace mint
