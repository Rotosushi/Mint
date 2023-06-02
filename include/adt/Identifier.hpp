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
#include <ostream>
#include <string>

namespace mint {

/*
  start = [a-zA-Z_];
  continue = [a-zA-Z0-9_];
  separator = "::";
  id= start continue* (separator continue+)*;
*/
class Identifier {
private:
  std::string data;

public:
  template <class... Args>
  Identifier(Args &&...args) noexcept : data{std::forward<Args>(args)} {}

  Identifier() noexcept {}
  Identifier(const Identifier &id) noexcept = default;
  Identifier(Identifier &&id) noexcept = default;
  auto operator=(const Identifier &id) noexcept -> Identifier & = default;
  auto operator=(Identifier &&id) noexcept -> Identifier & = default;

  auto view() const noexcept -> std::string_view { return data; }
  auto str() const noexcept -> std::string const & { return data; }
  auto empty() const noexcept -> bool { return data.empty(); }

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
  "x"              -> ""
  "::x"            -> ""
  "a::x"           -> "a"
  "a0::...::aN::x" -> "a0"
*/
  [[nodiscard]] auto first_scope() noexcept -> std::string_view {
    auto cursor = data.begin();
    auto end = data.end();
    while (cursor != end) {
      if (*cursor == ':') {
        return {data.data(), std::distance(data.begin(), cursor)};
      }

      cursor++;
    }

    return {""};
  }

  /*
    "x"              -> ""
    "::x"            -> ""
    "a::x"           -> ""
    "a0::...::aN::x" -> "a1::...::aN::x"
  */
  [[nodiscard]] auto rest_scope() noexcept -> std::string_view {
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
        return {cursor.base(), std::distance(cursor, end)};
      }

      cursor++;
    }

    return {""};
  }

  /*
    "x"              -> "x"
    "::x"            -> "x"
    "a::x"           -> "x"
    "a0::...::aN::x" -> "x"
  */
  [[nodiscard]] auto variable() noexcept -> std::string_view {
    // walk from the end of the identifier until
    // we see the first ':' character, then
    // return a view to the identifier one back
    // from ':'. if there is never a ':' simply
    // return the entire identifier.
    auto cursor = data.rbegin();
    auto end = data.rend();
    while (cursor != end) {
      if (*cursor == ':') {
        --cursor;
        return {cursor.base().base(), std::distance(cursor.base(), data.end())};
      }
      ++cursor;
    }

    return data;
  }
};

inline auto operator<<(std::ostream &out, Identifier const &id) noexcept
    -> std::ostream & {
  out << id.view();
  return out;
}

} // namespace mint

namespace std {
/*
  specialize std::hash to work with identifiers,
  so we can use identifiers directly in maps and sets
*/
template <> class hash<mint::Identifier> {
public:
  auto operator()(mint::Identifier const &id) const -> std::size_t {
    return std::hash<std::string>{}(id.str());
  }
};
} // namespace std
