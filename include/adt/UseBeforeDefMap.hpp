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
#include <map>
#include <optional>

#include "adt/UseBeforeDefNames.hpp"

#include "adt/Identifier.hpp"
#include "adt/Scope.hpp"
#include "ir/Mir.hpp"

namespace mint {
class Environment;

class UseBeforeDefMap {
public:
  struct Element {
    Identifier m_ubd_name;
    Identifier m_ubd_def_name;
    Identifier m_scope_name;
    ir::Mir m_def_ir;
    std::shared_ptr<Scope> m_scope;
    bool m_being_resolved;
  };
  using Elements = std::list<Element>;

  class iterator : public Elements::iterator {
  public:
    iterator(Elements::iterator iter) noexcept;

    [[nodiscard]] auto ubd_name() noexcept -> Identifier;
    [[nodiscard]] auto ubd_def_name() noexcept -> Identifier;
    [[nodiscard]] auto ubd_def_ir() noexcept -> ir::Mir &;
    [[nodiscard]] auto scope_name() noexcept -> Identifier;
    [[nodiscard]] auto scope() noexcept -> std::shared_ptr<Scope> &;
    [[nodiscard]] auto being_resolved() noexcept -> bool;
    auto being_resolved(bool state) noexcept -> bool;
  };

  class Range {
    std::vector<UseBeforeDefMap::iterator> m_range;
    using iterator = std::vector<UseBeforeDefMap::iterator>::iterator;

  public:
    [[nodiscard]] auto empty() const noexcept -> bool;
    void append(UseBeforeDefMap::iterator iter) noexcept;
    [[nodiscard]] auto begin() noexcept -> iterator;
    [[nodiscard]] auto end() noexcept -> iterator;
  };

private:
  Elements elements;

  [[nodiscard]] static auto contains_definition(Range &range, Identifier name,
                                                Identifier def_name) noexcept
      -> bool;

public:
  // lookup ubds in the map which are bound to the given name.
  // name is the name of the definition which was just created.
  // scope name is the name of the scope of the definition just created.
  [[nodiscard]] auto lookup(Identifier name) noexcept -> Range;

  void erase(iterator iter) noexcept;
  void erase(Range range) noexcept;

  void insert(Identifier ubd_name, Identifier ubd_def_name,
              Identifier scope_name, ir::Mir ir,
              std::shared_ptr<Scope> scope) noexcept;
  void insert(Element &&element) noexcept;
  void insert(Elements &&elements) noexcept;

  std::optional<Error> bindUseBeforeDef(Identifier undef, Identifier def,
                                        std::shared_ptr<Scope> const &scope,
                                        ir::Mir ir) noexcept;

public:
  std::optional<Error> resolveTypeOfUseBeforeDef(Environment &env,
                                                 Identifier def_name) noexcept;

  std::optional<Error>
  resolveComptimeValueOfUseBeforeDef(Environment &env,
                                     Identifier def_name) noexcept;

  std::optional<Error>
  resolveRuntimeValueOfUseBeforeDef(Environment &env,
                                    Identifier def_name) noexcept;
};
} // namespace mint
