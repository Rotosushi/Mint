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
#include "adt/Identifier.hpp"

namespace mint {
// #NOTE:
// if we consider a set of scopes (x0, x1, ..., xN)
// which are descending subscopes, defined as
// module x0 {
//   module x1 {
//     ...
//     module xN { ... } ... }}
// then a variable appearing in scope xA
// (where A is-in [0...N])
// is reachable by unqualified lookup from any scope xB
// (where B >= A and B <= N)

// if we consider a set of sets of scopes
// a0, a1, ..., aM
// where each (a0, a1, ..., aM)
// is-a set of descending subscopes (a0_0, a0_1, ..., a0_N)
// then a variable within scope xA_B
// (where A is-is [0...N] and B is-in [0...M])
// is reachable by unqualified lookup from any scope xC_D
// (where (A <= C <= N) and (B <= D <= M))

// additionally, any possible scope is a subscope of global scope

// that is, a variable is only reachable by unqualified lookup
// from a subscope of the defining scope of the variable.

// thus, this function returns true if and only if the qualifications
// of left are a subset of the qualifications of right.
auto subscopeOf(Identifier left, Identifier right) noexcept -> bool {
  auto q_left = left.qualifications();
  auto q_right = right.qualifications();
  return subscopeOf(q_left, q_right);
}

[[nodiscard]] auto subscopeOf(std::string_view left,
                              std::string_view right) noexcept -> bool {

  // if left is a longer string, then we know
  // left is more qualified than right, so it
  // cannot be a subscope of rights scope.
  if (left.size() > right.size())
    return false;

  auto l_cursor = left.begin();
  auto l_end = left.end();
  auto r_cursor = right.begin();
  // we know left <= right, so we only need to
  // look for the end of left.
  while (l_cursor != l_end) {
    // if the characters are not equal, then we
    // know that left is within a different scope
    // than right, at a less qualified scope than
    // right. meaning they are in parallel scopes.
    if (*l_cursor != *r_cursor)
      return false;

    ++l_cursor;
    ++r_cursor;
  }

  // we reach here in two cases, either the strings are equal,
  // in which case left is the same scope as right.
  // or they are equal for the entire length of left, which
  // means that right is more qualified than left,
  // meaning that right is a subscope of left.
  return true;
}

auto Identifier::globalNamespace() const noexcept -> Identifier {
  return set->empty_id();
}

[[nodiscard]] auto Identifier::qualifications() const noexcept
    -> std::string_view {
  // walk from the end until we see a ':'
  // if we do, eat the "::", then return an identifier from
  // the beginning of the string to that place
  auto cursor = data.rbegin();
  auto end = data.rend();
  while (cursor != end) {
    if (*cursor == ':') {
      ++cursor; // eat ':'
      // #NOTE: cursor now points to the first ':' within the last
      // "::" within the given identifier. this is the end of the
      // identifier representing the qualifications of this identifier.
      return {data.begin(), static_cast<std::string_view::size_type>(
                                std::distance(data.begin(), cursor.base()))};
    }

    ++cursor;
  }
  return set->empty_id();
}

[[nodiscard]] auto Identifier::firstScope() const noexcept -> Identifier {
  auto cursor = data.begin();
  auto end = data.end();
  while (cursor != end) {
    if (*cursor == ':') {
      return set->emplace(data.begin(),
                          static_cast<std::string_view::size_type>(
                              std::distance(data.begin(), cursor)));
    }

    ++cursor;
  }

  return set->empty_id();
}

[[nodiscard]] auto Identifier::restScope() const noexcept -> Identifier {
  // walk from the beginning of the view until the end,
  // if we see a "::" then walk just past the "::" and
  // return an identifier from there until the end of
  // the view, if we see no ':' then we simply return
  // a view from the cursor to the end. ("")
  auto cursor = data.begin();
  auto end = data.end();
  while (cursor != end) {
    if (*cursor == ':') {
      ++cursor; // eat ':'
      ++cursor; // eat ':'
      return set->emplace(cursor, static_cast<std::string_view::size_type>(
                                      std::distance(cursor, end)));
    }

    cursor++;
  }

  return *this;
}

[[nodiscard]] auto Identifier::variable() const noexcept -> Identifier {
  // walk from the end of the identifier until
  // we see the first ':' character, then
  // return a view to the identifier one back
  // from ':'. if there is never a ':' simply
  // return the entire identifier.
  auto cursor = data.rbegin();
  auto end = data.rend();
  while (cursor != end) {
    if (*cursor == ':') {
      //--cursor;
      return set->emplace(cursor.base(),
                          static_cast<std::string_view::size_type>(
                              std::distance(cursor.base(), data.end())));
    }
    ++cursor;
  }

  return *this;
}

[[nodiscard]] auto Identifier::prependScope(Identifier scope) const noexcept
    -> Identifier {
  // name doesn't begin with "::"
  if (!isGloballyQualified()) {
    std::string new_name{scope};
    new_name += "::";
    new_name += data;
    return set->emplace(std::move(new_name));
  }
  // data begins with "::", place the new scope qualifier
  // between the global qualifier and the rest of the qualifications
  std::string new_name{"::"};
  new_name += scope;
  new_name += data;
  return set->emplace(std::move(new_name));
}
} // namespace mint
