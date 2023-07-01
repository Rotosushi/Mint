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
#include <optional>
#include <ostream>
#include <string>
#include <system_error>
#include <variant>

#include "adt/Identifier.hpp"
#include "scan/Location.hpp"

namespace mint {
class Error {
public:
  struct Default {
    Location location;
    std::string message;
  };

  struct UseBeforeDef {
    Identifier def;
    Identifier q_def;
    Identifier undef;
    Identifier q_undef;
  };

  using Data = std::variant<std::monostate, Default, UseBeforeDef>;

  enum class Kind {
    // parser errors
    EndOfInput,

    UnknownToken,
    UnknownBinop,
    UnknownUnop,

    ExpectedADeclaration,
    ExpectedABasicTerm,
    ExpectedAType,
    ExpectedAnEquals,
    ExpectedASemicolon,
    ExpectedAnIdentifier,
    ExpectedAClosingParen,
    ExpectedABeginBrace,
    ExpectedAEndBrace,
    ExpectedText,

    // typecheck errors
    // import errors
    FileNotFound,
    ImportFailed,
    // definition errors
    LetTypeMismatch,

    UseBeforeDef,
    TypeCannotBeResolved,

    // variable errors
    NameUnboundInScope,
    NameAlreadyBoundInScope,
    NameIsPrivateInScope,

    // operator errors
    UnopTypeMismatch,
    BinopTypeMismatch,
  };

private:
  Kind m_kind;
  Data m_data;

  static auto KindToView(Kind kind) noexcept -> std::string_view;

public:
  Error(Kind kind) noexcept : m_kind(kind) {}
  Error(Kind kind, Location location, std::string_view message) noexcept
      : m_kind(kind),
        m_data(std::in_place_type<Default>, location, std::string(message)) {}
  Error(Kind kind, Identifier def, Identifier q_def, Identifier undef,
        Identifier q_undef) noexcept
      : m_kind(kind),
        m_data(std::in_place_type<UseBeforeDef>, def, q_def, undef, q_undef) {}

  [[nodiscard]] auto isMonostate() const noexcept -> bool {
    return std::holds_alternative<std::monostate>(m_data);
  }
  [[nodiscard]] auto isDefault() const noexcept -> bool {
    return std::holds_alternative<Default>(m_data);
  }
  [[nodiscard]] auto isUseBeforeDef() const noexcept -> bool {
    return std::holds_alternative<UseBeforeDef>(m_data);
  }

  [[nodiscard]] auto getDefault() const noexcept -> const Default & {
    MINT_ASSERT(isDefault());
    return std::get<Default>(m_data);
  }
  [[nodiscard]] auto getUseBeforeDef() const noexcept -> const UseBeforeDef & {
    MINT_ASSERT(isUseBeforeDef());
    return std::get<UseBeforeDef>(m_data);
  }

  static void underline(std::ostream &out, Location location,
                        std::string_view bad_source) noexcept {
    for (std::size_t i = 0; i <= bad_source.size(); ++i) {
      if ((i < location.fcolumn) || (i > location.lcolumn))
        out << " ";
      else
        out << "^";
    }
    out << "\n";
  }

  void print(std::ostream &out,
             std::string_view bad_source = "") const noexcept {
    out << KindToView(m_kind);

    // we don't print anything extra for other kinds of error
    if (std::holds_alternative<Default>(m_data)) {
      auto &default_data = std::get<Default>(m_data);

      auto &loc = default_data.location;
      auto &msg = default_data.message;

      out << " -- [" << loc.fline << ":" << loc.fcolumn << "]";
      out << " -- " << msg << "\n";

      if (!bad_source.empty()) {
        out << bad_source << "\n";
        underline(out, loc, bad_source);
      }
      out << "\n";
    }
  }

  auto kind() const noexcept { return m_kind; }
};

inline auto operator<<(std::ostream &out, Error &error) -> std::ostream & {
  error.print(out);
  return out;
}
} // namespace mint
