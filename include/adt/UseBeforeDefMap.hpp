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
#include <list>

#include "adt/Scope.hpp"
#include "adt/UseBeforeDefNames.hpp"
#include "ast/Ast.hpp"

namespace mint {

class UseBeforeDefMap {
public:
  using Element =
      std::tuple<UseBeforeDefNames, ast::Ptr, std::shared_ptr<Scope>>;
  using Elements = std::list<Element>;

  class iterator : public Elements::iterator {
  public:
    iterator(Elements::iterator iter) noexcept : Elements::iterator(iter) {}

    [[nodiscard]] auto names() noexcept -> UseBeforeDefNames & {
      return std::get<0>(**this);
    }
    [[nodiscard]] auto undef() noexcept -> Identifier { return names().undef; }
    [[nodiscard]] auto qualified_undef() noexcept -> Identifier {
      return names().qualified_undef;
    }
    [[nodiscard]] auto def() noexcept -> Identifier { return names().def; }
    [[nodiscard]] auto qualified_def() noexcept -> Identifier {
      return names().qualified_def;
    }
    [[nodiscard]] auto ast() noexcept -> ast::Ptr & {
      return std::get<1>(**this);
    }
    [[nodiscard]] auto scope() noexcept -> std::shared_ptr<Scope> & {
      return std::get<2>(**this);
    }
  };

  class Range {
    iterator m_first;
    iterator m_last;

  public:
    Range(iterator first, iterator last) noexcept
        : m_first(first), m_last(last) {}

    [[nodiscard]] auto begin() noexcept { return m_first; }
    [[nodiscard]] auto end() noexcept { return m_last; }
  };

private:
  Elements elements;

public:
  [[nodiscard]] auto lookup(Identifier undef) noexcept -> Range {
    auto names_match = [](iterator cursor, Identifier undef) {
      return (cursor.undef() == undef) || (cursor.qualified_undef() == undef);
    };

    iterator cursor = elements.begin();
    iterator end = elements.end();
    while (cursor != end) {
      if (names_match(cursor, undef)) {
        iterator range_end = cursor;
        do {
          ++range_end;
        } while (names_match(cursor, undef) && (range_end != end));
        return {cursor, range_end};
      }

      ++cursor;
    }
    return {end, end};
  }

  void erase(iterator iter) noexcept {
    if (iter != elements.end())
      elements.erase(iter);
  }
  void erase(Range range) noexcept {
    elements.erase(range.begin(), range.end());
  }

  void insert(UseBeforeDefNames names, ast::Ptr ast,
              std::shared_ptr<Scope> scope) noexcept {
    // #NOTE: this is considered a local use-before-def,
    // so we bind it to the unqualified undef name.
    // #NOTE: we allow multiple definitions to be bound to
    // the same undef name, however we want to prevent the
    // same definition being bound in the table under the
    // same undef name twice.
    auto range = lookup(names.undef);
    auto cursor = range.begin();
    auto end = range.end();
    while (cursor != end) {
      if (cursor.def() == names.def)
        return;

      ++cursor;
    }

    // #NOTE that due to the above check, iff we reach here
    // then cursor points to the end of the equal range of
    // values stored within the map. thus we can simply
    // insert there to maintain the equal range.
    elements.emplace(cursor, names, std::move(ast), scope);
  }
};

} // namespace mint
