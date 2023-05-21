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
#include <expected>
#include <optional>
#include <ostream>
#include <string>

#include "scan/Location.hpp"

namespace mint {
class Error {
public:
  enum Kind {
    UnknownToken,
    UnknownBinop,

    ExpectedABasicTerm,
    ExpectedAnEquals,
    ExpectedASemicolon,
    ExpectedAnIdentifier,
  };

private:
  Kind kind;
  std::optional<Location> location;
  std::optional<std::string> message;

  static auto KindToSV(Kind kind) noexcept -> std::string_view;

public:
  Error(Kind kind) noexcept
      : kind(kind), location(std::nullopt), message(std::nullopt) {}
  Error(Kind kind, Location location, std::string_view message) noexcept
      : kind(kind), location(location), message(message) {}

  void print(std::ostream &out) const noexcept {
    out << KindToSV(kind);

    if (location.has_value()) {
      auto &loc = location.value();
      out << " -- [" << loc.fline << ":" << loc.fcolumn << "]";
    }

    if (message.has_value()) {
      out << " --  " << message.value() << "\n";
    }
  }
};

inline auto operator<<(std::ostream &out, Error &error) -> std::ostream & {
  error.print(out);
  return out;
}

template <class T> using expected = std::expected<T, Error>;

} // namespace mint
