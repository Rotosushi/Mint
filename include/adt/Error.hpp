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

#include "adt/SourceLocation.hpp"
#include "adt/UseBeforeDefNames.hpp"
#include "scan/Location.hpp"

// #NOTE: Errors are currently returned by value in most cases.
// this works fine for single errors. however I think it would
// be interesting and useful to create a list of errors within
// the environment, create errors and place them into that list
// then return a reference to that error, instead of the error
// itself.
// this has the major benefiet of reducing the amount of copying
// of errors during parsing, typechecking, evaluation, and codegen.
// #TODO: add a ErrorList (or similar) data structure to the
// environment. to hold errors as they are generated.

namespace mint {
class Scope;
// #TODO: make errors cheap to pass around, by
// storing their actual contents in a list somewhere else.

// Represents a error which occurs during compilation that is
// relevant to the programer
class Error {
public:
  enum class Kind {
    Default,

    // parser errors
    EndOfInput,

    UnknownToken,
    UnknownBinop,
    UnknownUnop,

    ExpectedBasic,
    ExpectedType,
    ExpectedEquals,
    ExpectedColon,
    ExpectedSemicolon,
    ExpectedBackSlash,
    ExpectedIdentifier,
    ExpectedBeginParen,
    ExpectedEndParen,
    ExpectedBeginBrace,
    ExpectedEndBrace,
    ExpectedRightArrow,
    ExpectedEqualsRightArrow,
    ExpectedText,
    ExpectedDefinition,
    ExpectedVisibility,
    ExpectedKeywordLet,
    ExpectedKeywordFn,
    ExpectedKeywordModule,
    ExpectedKeywordImport,

    // typecheck errors
    // import errors
    FileNotFound,
    ImportFailed,

    // definition errors
    AnnotatedTypeMismatch,

    UseBeforeDef,
    TypeCannotBeResolved,

    // variable errors
    NameUnboundInScope,
    NameAlreadyBoundInScope,
    NameIsPrivateInScope,

    // Function Errors
    ResultTypeMismatch,
    MainReturnTypeMismatch,
    MainAnnotatedTypeMismatch,
    MainArgumentTypesMismatch,

    // Call errors
    CannotCallType,
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
  std::optional<SourceLocation *> m_sl;
  std::optional<std::string> m_message;

  static auto KindToView(Kind kind) noexcept -> std::string_view;

public:
  Error(Kind kind, std::optional<SourceLocation *> location = std::nullopt,
        std::optional<std::string_view> message = std::nullopt) noexcept;

  static void underline(std::ostream &out, Location location,
                        std::string_view bad_source) noexcept;

  void print(std::ostream &out) const noexcept;

  auto kind() const noexcept -> Kind;
};

inline auto operator<<(std::ostream &out, Error const &error)
    -> std::ostream & {
  error.print(out);
  return out;
}
} // namespace mint
