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
#include <string_view>

namespace mint {
class Identifier {
private:
  std::string_view view;

  Identifier(std::string_view view) noexcept : view{view} {}

public:
  Identifier(const Identifier &id) noexcept = default;
  Identifier(Identifier &&id) noexcept = default;
  auto operator=(const Identifier &id) noexcept -> Identifier & = default;
  auto operator=(Identifier &&id) noexcept -> Identifier & = default;

  operator std::string_view() const noexcept { return view; }
  auto get() const noexcept -> std::string_view { return view; }

  auto operator==(const Identifier &other) const noexcept -> bool {
    return view.data() == other.view.data();
  }

  friend class IdentifierSet;
};

inline auto operator<<(std::ostream &out, Identifier id) noexcept
    -> std::ostream & {
  out << id.get();
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
  auto operator()(const mint::Identifier &id) const -> std::size_t {
    return std::hash<std::string_view>{}(id);
  }
};
} // namespace std
