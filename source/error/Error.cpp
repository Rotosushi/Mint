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
#include "error/Error.hpp"

#include "utility/FatalError.hpp"

namespace mint {
auto Error::KindToSV(Error::Kind kind) noexcept -> std::string_view {
  switch (kind) {
  case Error::UnknownToken:
    return "Unknown Token";
  case Error::UnknownBinop:
    return "Unknown Binop";

  case Error::ExpectedABasicTerm:
    return "Expected a basic term";
  case Error::ExpectedAnEquals:
    return "Expected a '='";
  case Error::ExpectedASemicolon:
    return "Expected a ';'";
  case Error::ExpectedAnIdentifier:
    return "Expected an identifier";

  default:
    fatalError("Unknown Error::Kind");
  }
}
} // namespace mint
