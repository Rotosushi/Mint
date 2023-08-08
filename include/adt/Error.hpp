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
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <system_error>
#include <variant>

#include "adt/UseBeforeDefNames.hpp"
#include "scan/Location.hpp"

namespace mint {
class Scope;
class Error {
public:
  struct Default {
    Location location;
    std::string message;
  };

  struct UseBeforeDef {
    UseBeforeDefNames names;
    std::shared_ptr<Scope> scope;
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
    ExpectedAColon,
    ExpectedASemicolon,
    ExpectedAnIdentifier,
    ExpectedAClosingParen,
    ExpectedABeginBrace,
    ExpectedAEndBrace,
    ExpectedARightArrow,
    ExpectedAEqualsRightArrow,
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

    // Function Errors
    ResultTypeMismatch,

    // Call errors
    CannotCallObject,
    ArgumentTypeMismatch,
    ArgumentNumberMismatch,

    // operator errors
    UnopTypeMismatch,
    BinopTypeMismatch,

    // codegen errors
    GlobalInitNotConstant,
  };

private:
  Kind m_kind;
  Data m_data;

  static auto KindToView(Kind kind) noexcept -> std::string_view;

public:
  Error(Kind kind) noexcept;
  Error(Kind kind, Location location, std::string_view message) noexcept;
  Error(Kind kind, UseBeforeDefNames names,
        std::shared_ptr<Scope> scope) noexcept;
  Error(UseBeforeDef const &usedef) noexcept;

  [[nodiscard]] auto isMonostate() const noexcept -> bool;
  [[nodiscard]] auto isDefault() const noexcept -> bool;
  [[nodiscard]] auto isUseBeforeDef() const noexcept -> bool;

  [[nodiscard]] auto getDefault() const noexcept -> const Default &;
  [[nodiscard]] auto getUseBeforeDef() const noexcept -> const UseBeforeDef &;

  static void underline(std::ostream &out, Location location,
                        std::string_view bad_source) noexcept;

  void print(std::ostream &out,
             std::string_view bad_source = "") const noexcept;

  auto kind() const noexcept -> Kind;
};

inline auto operator<<(std::ostream &out, Error &error) -> std::ostream & {
  error.print(out);
  return out;
}
} // namespace mint
