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
#include <string_view>
#include <unordered_set>

namespace mint {
class InternedString {
private:
  std::string_view view;

public:
  InternedString(std::string_view view) noexcept : view{view} {}

  operator std::string_view() noexcept { return view; }
  auto get() const noexcept -> std::string_view { return view; }

  auto operator==(const InternedString &other) const noexcept -> bool {
    return view.data() == other.view.data();
  }
};

class StringSet {
private:
  std::unordered_set<std::string_view> set;

public:
  [[nodiscard]] auto emplace(std::string_view view) noexcept -> InternedString {
    auto found = set.find(view);
    if (found != set.end()) {
      return *found;
    }

    auto pair = set.emplace(view);
    return *pair.first;
  }
};
} // namespace mint
