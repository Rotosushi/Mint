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

// #NOTE: this brings up the idea of the success path and
// failure path of the compiler. currently, each major pass
// of the compiler; (If i am using 'pass' correctly as understood
// by other languages) parse, typecheck, evaluate, codegen,
// are defined by their success path and failure path. and this
// behavior is encapsulated within the Result<T, E> class.
// which holds the success or failure of the function call,
// and returns the data relevant to either path.
// What I am coming up to is noticing the structure
// as it relates to UseBeforeDef errors. as of now, UseBeforeDef
// is an Error, and it is handled on the failure path of the compiler.
// however UBD is not handled the same way as other errors, and as such
// it adds a layer of complexity to the failure path.
// This is tolerable only in so far as there is only one kind of
// non-error error. if there is more than one, Then an abstraction
// is needed to encapsulate this non-error, non-success path and
// it's set of values. Then these things can be handled more
// systematically. An initial guess would simply be to modify
// Result<T, E> to Result<T, E, U> where T is the success path
// E is the error path, and U is the recoverable path.
// (though maybe T, F, U is clearer? it's E for Error btw.
// T = true, F = false, U = unknown)
// the U path encapsulates the idea that while computation cannot
// continue down the path it took, the success or failure of the
// expression has not been decided.
// variable use before definition is an example of the U path.
// The compiler cannot decide if the variables use is incorrect
// or correct until at a point later in the process.
// another example of a U path is runtime only expressions within
// the comptime context. Runtime only expressions are both possible
// and necessary, and yet sometimes there are contexts where their
// use must be disallowed. this is allowed in the general sense,
// as it does not hamper codegeneration, however in a strict compile time
// context this is raised to an error.
// #NOTE: the way that I want to describe this third path is a
// recoverable error. so maybe that should be a 'kind' or 'category' of error.
// and not handled within the result class.
// though in the result class it is much easier the not nest the
// code handling the recoverable error within the scope of the
// code handling unrecoverable errors. especially when using
// conditional continues within loops. (which we do in the repl,
// and import mechanism)

namespace mint {
class Scope;
// #TODO: refactor to use std::expected, as we no longer track
// UBD here.

// Represents a error which occurs during compilation that is
// relevant to the programer
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
