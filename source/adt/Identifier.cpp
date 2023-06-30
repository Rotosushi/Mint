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
[[nodiscard]] auto Identifier::first_scope() noexcept -> Identifier {
  auto cursor = data.begin();
  auto end = data.end();
  while (cursor != end) {
    if (*cursor == ':') {
      return set->emplace(data.begin(),
                          static_cast<std::string_view::size_type>(
                              std::distance(data.begin(), cursor)));
    }

    cursor++;
  }

  return set->emplace("");
}

[[nodiscard]] auto Identifier::rest_scope() noexcept -> Identifier {
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

  return set->emplace("");
}

[[nodiscard]] auto Identifier::variable() noexcept -> Identifier {
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
      return set->emplace(cursor.base(),
                          static_cast<std::string_view::size_type>(
                              std::distance(cursor.base(), data.end())));
    }
    ++cursor;
  }

  return *this;
}

[[nodiscard]] auto Identifier::prependScope(Identifier scope) noexcept
    -> Identifier {
  if (!globallyQualified()) {
    std::string new_name{scope};
    new_name += "::";
    new_name += data;
    return set->emplace(std::move(new_name));
  }
  return *this;
}
} // namespace mint
