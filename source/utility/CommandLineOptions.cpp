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
#include "utility/CommandLineOptions.hpp"

#include "utility/Config.hpp"

namespace mint {
void printVersion(llvm::raw_ostream &out) noexcept {
  out << "mint version: " << MINT_VERSION_MAJOR << "." << MINT_VERSION_MINOR
      << "." << MINT_VERSION_PATCH << "\n git revision [" << MINT_GIT_REVISION
      << "]\n Compiled on " << __DATE__ << " at " << __TIME__ << "\n";
}
} // namespace mint
