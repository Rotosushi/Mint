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
#include <cstring>
#include <ostream>

#include "adt/StringSet.hpp"

namespace mint {

//   start = [a-zA-Z_];
//   continue = [a-zA-Z0-9_];
//   separator = "::";
//   id= start continue* (separator continue+)*;
class Identifier {
private:
  StringSet *set;
  std::string_view data;

  Identifier(StringSet *set, std::string_view data) noexcept;

public:
  Identifier(const Identifier &id) noexcept = default;
  Identifier(Identifier &&id) noexcept = default;
  auto operator=(const Identifier &id) noexcept -> Identifier & = default;
  auto operator=(Identifier &&id) noexcept -> Identifier & = default;

  static Identifier create(StringSet *set, std::string_view string) noexcept;

  auto operator==(const Identifier &other) const noexcept -> bool;
  operator std::string_view() const noexcept;
  auto view() const noexcept -> std::string_view;
  auto getSet() const noexcept { return set; }
  auto empty() const noexcept -> bool;
  auto globalQualification() const noexcept -> Identifier;

  // "x"              -> false
  // "::x"            -> true
  // "a::x"           -> true
  // "a0::...::aN::x" -> true
  [[nodiscard]] auto isQualified() const noexcept -> bool;

  // does this identifier begin with global scope?
  // "x"              -> false
  // "::x"            -> true
  // "a::x"           -> false
  // "a0::...::aN::x" -> false
  [[nodiscard]] auto isGloballyQualified() const noexcept -> bool;

  // #WARNING: qualifications does not return a valid Identifier
  // because the qualifications of an Identifier are not
  // themselves a valid identifier, from the perspective of lookup.
  // #NOTE: this function assumes that it is given a valid
  // Identifier
  //  "x"              -> ""
  //  "::x"            -> ""
  //  "a::x"           -> "a"
  //  "a0::...::aN::x" -> "a0::...::aN"
  [[nodiscard]] auto qualifications() const noexcept -> Identifier;

  //  "x"              -> ""
  //  "::x"            -> ""
  //  "a::x"           -> "a"
  //  "a0::...::aN::x" -> "a0"
  [[nodiscard]] auto firstScope() const noexcept -> Identifier;

  //   "x"              -> ""
  //   "::x"            -> ""
  //   "a::x"           -> ""
  //   "a0::...::aN::x" -> "a1::...::aN::x"
  [[nodiscard]] auto restScope() const noexcept -> Identifier;

  //   "x"              -> "x"
  //   "::x"            -> "x"
  //   "a::x"           -> "x"
  //   "a0::...::aN::x" -> "x"
  [[nodiscard]] auto variable() const noexcept -> Identifier;

  //   "x",   "a"           -> "a::x"
  //   "::x", "a"           -> "::x"
  //   "a::x", "b"          -> "b::a::x"
  //   "a0::...::aN::x"     -> "b::a0::...::aN::x"
  [[nodiscard]] auto prependScope(Identifier scope) const noexcept
      -> Identifier;

  // https://llvm.org/docs/LangRef.html#identifiers
  [[nodiscard]] auto convertForLLVM() const noexcept -> Identifier;
};

inline auto operator<<(std::ostream &out, Identifier const &id) noexcept
    -> std::ostream & {
  out << id.view();
  return out;
}

// #NOTE: does name appear in a scope which may be
// reached by unqualified lookup from scope?
// this function answers the question:
//  is scope a subscope of the scope of name?
[[nodiscard]] auto subscopeOf(Identifier scope, Identifier name) noexcept
    -> bool;
} // namespace mint

namespace std {
// specialize std::less to work with Identifiers,
// so we can use them as keys in ordered maps and sets.
template <> struct less<mint::Identifier> {
  auto operator()(mint::Identifier const &l, mint::Identifier const &r) const
      -> bool {
    return l.view() < r.view();
  }
};

//  specialize std::hash to work with identifiers,
//  so we can use identifiers as keys in unordered maps and sets
template <> struct hash<mint::Identifier> {
  auto operator()(mint::Identifier const &id) const -> std::size_t {
    return std::hash<std::string_view>{}(id.view());
  }
};

} // namespace std
