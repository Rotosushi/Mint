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
#include <string>
#include <unordered_set>

#include "utility/Allocator.hpp"
#include "utility/Assert.hpp"

namespace mint {
class Identifier;

class IdentifierSet {
private:
  std::unordered_set<std::string, std::hash<std::string>,
                     std::equal_to<std::string>, PolyAllocator<std::string>>
      set;

public:
  IdentifierSet(Allocator &allocator) noexcept : set(allocator) {}

  template <class... Args>
  [[nodiscard]] auto emplace(Args &&...args) noexcept -> Identifier;
};

/*
  start = [a-zA-Z_];
  continue = [a-zA-Z0-9_];
  separator = "::";
  id= start continue* (separator continue+)*;
*/
class Identifier {
private:
  IdentifierSet *set;
  std::string_view data;

  Identifier(IdentifierSet *set, std::string_view data) noexcept
      : set(set), data(data) {
    MINT_ASSERT(set != nullptr);
  }

  friend class IdentifierSet;

public:
  Identifier(const Identifier &id) noexcept = default;
  Identifier(Identifier &&id) noexcept = default;
  auto operator=(const Identifier &id) noexcept -> Identifier & = default;
  auto operator=(Identifier &&id) noexcept -> Identifier & = default;

  // since identifiers are interned, comparison can
  // be done via pointer comparison.
  auto operator==(const Identifier &other) const noexcept -> bool {
    return data.begin() == other.data.begin();
  }

  operator std::string_view() const noexcept { return data; }
  auto view() const noexcept -> std::string_view { return data; }
  auto empty() const noexcept -> bool { return data.empty(); }
  // auto get(std::string_view data) noexcept -> Identifier;

  /*
  does this identifier begin with a scope?

  "x"              -> false
  "::x"            -> true
  "a::x"           -> true
  "a0::...::aN::x" -> true
  */
  [[nodiscard]] auto isScoped() const noexcept -> bool {
    for (auto c : data) {
      if (c == ':')
        return true;
    }
    return false;
  }

  /*
  does this identifier begin with global scope?

  "x"              -> false
  "::x"            -> true
  "a::x"           -> false
  "a0::...::aN::x" -> false
  */
  [[nodiscard]] auto globallyQualified() const noexcept -> bool {
    if (*data.begin() == ':') {
      return true;
    }
    return false;
  }

  /*
  "x"              -> ""
  "::x"            -> ""
  "a::x"           -> "a"
  "a0::...::aN::x" -> "a0"
*/
  [[nodiscard]] auto first_scope() noexcept -> Identifier;

  /*
    "x"              -> ""
    "::x"            -> ""
    "a::x"           -> ""
    "a0::...::aN::x" -> "a1::...::aN::x"
  */
  [[nodiscard]] auto rest_scope() noexcept -> Identifier;

  /*
    "x"              -> "x"
    "::x"            -> "x"
    "a::x"           -> "x"
    "a0::...::aN::x" -> "x"
  */
  [[nodiscard]] auto variable() noexcept -> Identifier;

  /*
    "x",   "a"           -> "a::x"
    "::x", "a"           -> "::x"
    "a::x", "b"          -> "b::a::x"
    "a0::...::aN::x"     -> "b::a0::...::aN::x"
  */
  [[nodiscard]] auto prependScope(Identifier scope) noexcept -> Identifier;
};

inline auto operator<<(std::ostream &out, Identifier const &id) noexcept
    -> std::ostream & {
  out << id.view();
  return out;
}

template <class... Args>
[[nodiscard]] inline auto IdentifierSet::emplace(Args &&...args) noexcept
    -> Identifier {
  auto pair = set.emplace(std::forward<Args>(args)...);
  return Identifier(this, *pair.first);
}

} // namespace mint

namespace std {
template <> struct less<mint::Identifier> {
  auto operator()(mint::Identifier const &l, mint::Identifier const &r) const
      -> bool {
    auto left = l.view();
    auto right = r.view();
    auto llen = left.length();
    auto rlen = right.length();
    // is left is a shorter string, it's less than
    if (llen < rlen) {
      return true;
    } // if left is a longer string, it's not less than
    else if (llen > rlen) {
      return false;
    } else { // llen == rlen
      return strncmp(left.begin(), right.begin(), llen);
    }
  }
};
/*
  specialize std::hash to work with identifiers,
  so we can use identifiers directly in maps and sets
*/
template <> struct hash<mint::Identifier> {
  auto operator()(mint::Identifier const &id) const -> std::size_t {
    return std::hash<std::string_view>{}(id.view());
  }
};

} // namespace std
