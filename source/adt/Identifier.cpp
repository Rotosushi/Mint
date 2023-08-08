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
[[nodiscard]] auto IdentifierSet::empty_id() noexcept -> Identifier {
  return emplace("");
}

Identifier::Identifier(IdentifierSet *set, std::string_view data) noexcept
    : set(set), data(data) {
  MINT_ASSERT(set != nullptr);
}

// #NOTE: since identifiers are interned, comparison can be
// implemented as pointer comparison.
auto Identifier::operator==(const Identifier &other) const noexcept -> bool {
  return data.begin() == other.data.begin();
}

Identifier::operator std::string_view() const noexcept { return data; }

auto Identifier::view() const noexcept -> std::string_view { return data; }

auto Identifier::empty() const noexcept -> bool { return data.empty(); }

auto Identifier::globalQualification() const noexcept -> Identifier {
  return set->empty_id();
}

[[nodiscard]] auto Identifier::isQualified() const noexcept -> bool {
  for (auto c : data) {
    if (c == ':')
      return true;
  }
  return false;
}

[[nodiscard]] auto Identifier::isGloballyQualified() const noexcept -> bool {
  if (*data.begin() == ':') {
    return true;
  }
  return false;
}

[[nodiscard]] auto Identifier::qualifications() const noexcept -> Identifier {
  // walk from the end until we see a ':'
  // if we do, eat the "::", then return an identifier from
  // the beginning of the string to that place
  auto cursor = data.rbegin();
  auto end = data.rend();
  while (cursor != end) {
    if (*cursor == ':') {
      ++cursor; // eat ':'
      ++cursor; // eat ':'
      return set->emplace(data.begin(),
                          static_cast<std::string_view::size_type>(
                              std::distance(data.begin(), cursor.base())));
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

/* https://llvm.org/docs/LangRef.html#identifiers */
[[nodiscard]] auto Identifier::convertForLLVM() const noexcept -> Identifier {
  std::string llvm_name;
  // the llvm_name is the same as the given name,
  // where "::" is replaced with "."
  auto cursor = data.begin();
  auto end = data.end();
  while (cursor != end) {
    auto c = *cursor;
    if (c == ':') {
      llvm_name += '.';
      ++cursor; // eat "::"
      ++cursor;
    } else {
      llvm_name += c;
      ++cursor; // eat the char
    }
  }

  return set->emplace(std::move(llvm_name));
}

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
// of scope are a subset of the qualifications of name.
auto subscopeOf(Identifier scope, Identifier name) noexcept -> bool {
  auto q_name = name.qualifications();
  // if the qualifications of name is a longer string,
  // than the scope. then we know name is more qualified
  // than the scope, so it cannot be a subscope of left.
  if (q_name.view().size() > scope.view().size())
    return false;

  // we know right <= left, so we only need to
  // look for the end of right.
  auto s_cursor = scope.view().begin();
  auto n_cursor = q_name.view().begin();
  auto n_end = q_name.view().end();
  while (n_cursor != n_end) {
    // if the characters are not equal, then we
    // know that the scopes diverge at a point
    // before they reach their full specificity.
    // meaning they are parallel scopes.
    if (*s_cursor != *n_cursor)
      return false;

    ++s_cursor;
    ++n_cursor;
  }

  // we reach here in two cases, either the strings are equal,
  // in which case scope is the same scope as name's scope.
  // meaning they are subscopes. (because a set is a subset of itself)
  // or they are equal for the entire length of names
  // qualifications, which means that scope is more qualified
  // than the scope of name meaning that scope is a subscope of
  // names scope.
  return true;
}

} // namespace mint
