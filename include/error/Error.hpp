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
    EndOfInput,

    UnknownToken,
    UnknownBinop,
    UnknownUnop,

    ExpectedABasicTerm,
    ExpectedAnEquals,
    ExpectedASemicolon,
    ExpectedAnIdentifier,
    ExpectedAClosingParen,

    NameUnboundInScope,
    NameAlreadyBoundInScope,

    UnopTypeMismatch,
    BinopTypeMismatch,
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

  void underline(std::ostream &out,
                 std::string_view bad_source) const noexcept {
    if (!location.has_value()) {
      return;
    }
    auto loc = location.value();

    for (std::size_t i = 0; i <= bad_source.size(); ++i) {
      if ((i < loc.fcolumn) || (i > loc.lcolumn))
        out << " ";
      else
        out << "^";
    }
    out << "\n";
  }

  void print(std::ostream &out,
             std::string_view bad_source = "") const noexcept {
    out << KindToSV(kind);

    if (location.has_value()) {
      auto &loc = location.value();
      out << " -- [" << loc.fline << ":" << loc.fcolumn << "]";
    }

    if (message.has_value()) {
      out << " --  " << message.value() << "\n";
    }

    if (!bad_source.empty()) {
      out << bad_source << "\n";
      underline(out, bad_source);
    }
  }

  auto getLocation() const noexcept { return location; }
  auto getMessage() const noexcept -> std::optional<std::string_view> {
    if (message.has_value())
      return static_cast<std::string_view>(message.value());
    else
      return {};
  }
};

inline auto operator<<(std::ostream &out, Error &error) -> std::ostream & {
  error.print(out);
  return out;
}

template <class T> class Result {
  std::expected<T, Error> data;

public:
  Result(T t) noexcept : data(std::move(t)) {}
  Result(Error &&e) noexcept : data(std::unexpect, std::move(e)) {}
  Result(Error::Kind kind) noexcept : data(std::unexpect, kind) {}
  Result(Error::Kind kind, Location location, std::string_view message) noexcept
      : data(std::unexpect, kind, location, message) {}

  operator bool() const noexcept { return data.has_value(); }

  [[nodiscard]] auto has_value() const noexcept -> bool {
    return data.has_value();
  }

  [[nodiscard]] auto value() noexcept -> T & {
    MINT_ASSERT(data.has_value());
    return data.value();
  }
  [[nodiscard]] auto error() noexcept -> Error & {
    MINT_ASSERT(!data.has_value());
    return data.error();
  }
};

} // namespace mint
