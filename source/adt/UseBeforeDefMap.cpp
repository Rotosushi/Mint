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
#include "adt/UseBeforeDefMap.hpp"

namespace mint {
UseBeforeDefMap::iterator::iterator(Elements::iterator iter) noexcept
    : Elements::iterator(iter) {}

[[nodiscard]] auto UseBeforeDefMap::iterator::ubd_name() noexcept
    -> Identifier {
  return (*this)->m_ubd_name;
}
[[nodiscard]] auto UseBeforeDefMap::iterator::ubd_def_name() noexcept
    -> Identifier {
  return (*this)->m_ubd_def_name;
}
[[nodiscard]] auto UseBeforeDefMap::iterator::ubd_def_ast() noexcept
    -> ast::Ptr & {
  return (*this)->m_ubd_def_ast;
}
[[nodiscard]] auto UseBeforeDefMap::iterator::scope_name() noexcept
    -> Identifier {
  return (*this)->m_scope_name;
}
[[nodiscard]] auto UseBeforeDefMap::iterator::scope() noexcept
    -> std::shared_ptr<Scope> & {
  return (*this)->m_scope;
}
[[nodiscard]] auto UseBeforeDefMap::iterator::being_resolved() noexcept
    -> bool {
  return (*this)->m_being_resolved;
}
auto UseBeforeDefMap::iterator::being_resolved(bool state) noexcept -> bool {
  return ((*this)->m_being_resolved = state);
}

[[nodiscard]] auto UseBeforeDefMap::Range::empty() const noexcept -> bool {
  return m_range.empty();
}
void UseBeforeDefMap::Range::append(UseBeforeDefMap::iterator iter) noexcept {
  m_range.push_back(iter);
}
[[nodiscard]] auto UseBeforeDefMap::Range::begin() noexcept
    -> UseBeforeDefMap::Range::iterator {
  return m_range.begin();
}
[[nodiscard]] auto UseBeforeDefMap::Range::end() noexcept
    -> UseBeforeDefMap::Range::iterator {
  return m_range.end();
}

[[nodiscard]] auto
UseBeforeDefMap::contains_definition(Range &range, Identifier name,
                                     Identifier def_name) noexcept -> bool {
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    auto it = *cursor;
    if ((it.ubd_name() == name) && (it.ubd_def_name() == def_name))
      return true;

    ++cursor;
  }
  return false;
}

[[nodiscard]] auto UseBeforeDefMap::lookup(Identifier name) noexcept -> Range {
  Range result;

  iterator cursor = elements.begin();
  iterator end = elements.end();
  while (cursor != end) {
    // iff this cursor is currently being resolved, we
    // don't want to match on it. as this causes erroneous
    // symbol redefinition errors.
    if (cursor.being_resolved()) {
      ++cursor; // #NOTE: we ++cursor because the continue skips
      continue; // the ++cursor at the end of the loop body.
    }

    auto ubd_name = cursor.ubd_name();
    auto ubd_def_name = cursor.ubd_def_name();
    auto scope_name = cursor.scope_name();
    if (ubd_name == name) {
      result.append(cursor);
      ++cursor;
      continue;
    }

    // the name doesn't match. however, if the scope of the
    // ubd definition is a subscope of the definition that
    // was just created, then unqualified lookup from the
    // ubd definition's scope will resolve the definition
    // that was just created, thus we can rely on the
    // weaker comparison of the unqualified names
    // to resolve use before definition.
    if (subscopeOf(scope_name, name)) {
      auto unqualified_ubd_name = ubd_name.variable();
      auto unqualified_name = name.variable();
      if (unqualified_ubd_name == unqualified_name) {
        result.append(cursor);
        ++cursor;
        continue;
      }
      // else fallthrough
    }

    // the name won't be resolved through unqualified lookup,
    // and the name doesn't match directly, however, we could
    // still resolve by qualified lookup. so if name is qualified,
    // then we want to resolve names in the map which are
    // dependant on that name through qualified lookup.
    // we assume what is looked up in the map is fully qualified,
    // and we assume that the ubd_name is qualified such that
    // it would resolve to the new definition from the scope
    // in which the ubd_definition was defined, all we need to do
    // is search for the composite name
    if (name.isQualified()) {
      auto local_qualifications = ubd_def_name.qualifications();
      auto locally_qualified_name =
          local_qualifications.empty()
              ? ubd_name
              : ubd_name.prependScope(local_qualifications);
      if (name == locally_qualified_name) {
        result.append(cursor);
        ++cursor;
        continue;
      }
      // else fallthrough
    }

    ++cursor;
  }
  return result;
}

void UseBeforeDefMap::erase(iterator iter) noexcept {
  if (iter != elements.end())
    elements.erase(iter);
}
void UseBeforeDefMap::erase(Range range) noexcept {
  for (auto it : range) {
    erase(it);
  }
}

void UseBeforeDefMap::insert(Identifier ubd_name, Identifier ubd_def_name,
                             Identifier scope_name, ast::Ptr ast,
                             std::shared_ptr<Scope> scope) noexcept {
  // #NOTE: we allow multiple definitions to be bound to
  // the same undef name, however we want to prevent the
  // same definition being bound in the table under the
  // same undef name twice.
  auto range = lookup(ubd_name);
  if (contains_definition(range, ubd_name, ubd_def_name))
    return;

  elements.emplace(elements.end(), ubd_name, ubd_def_name, scope_name,
                   std::move(ast), scope, false);
}

void UseBeforeDefMap::insert(Element &&element) noexcept {
  auto range = lookup(element.m_ubd_name);
  if (contains_definition(range, element.m_ubd_name, element.m_ubd_def_name))
    return;

  elements.emplace(elements.end(), std::move(element));
}

void UseBeforeDefMap::insert(Elements &&elements) noexcept {
  for (auto &&element : elements)
    insert(std::move(element));
}
} // namespace mint