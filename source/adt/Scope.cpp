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
#include "adt/Scope.hpp"

namespace mint {
[[nodiscard]] auto ScopeTable::Entry::ptr() const noexcept
    -> std::shared_ptr<Scope> {
  return iter->second->shared_from_this();
}

[[nodiscard]] auto ScopeTable::Entry::namesEmpty() const noexcept -> bool {
  return iter->second->bindingsEmpty();
}

[[nodiscard]] auto ScopeTable::Entry::scopesEmpty() const noexcept -> bool {
  return iter->second->scopesEmpty();
}

auto ScopeTable::Entry::bind(Identifier name, Attributes attributes,
                             Type::Pointer type, Ast::Ptr value) noexcept
    -> Bindings::Binding {
  return iter->second->bindName(std::move(name), attributes, type, value);
}

[[nodiscard]] auto ScopeTable::Entry::lookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->lookup(name);
}

auto ScopeTable::emplace(Identifier name,
                         std::weak_ptr<Scope> prev_scope) noexcept -> Entry {
  auto pair = table.emplace(name, Scope::createScope(name, prev_scope));
  return pair.first;
}

/*
  lookup scope "a0" from name of the form "a0::...::aN::x"
*/
[[nodiscard]] auto Scope::qualifiedScopeLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  /*
    if name begins with "::"
  */
  if (name.globallyQualified()) {
    auto g = global.lock();
    return g->qualifiedLookup(name.variable());
  }

  /*
    "a0::...::aN::x" -> "a0"
  */
  auto first = name.first_scope();
  auto scope = scopes.lookup(first);
  if (!scope) {
    // lookup in the above scope.
    if (!isGlobal()) {
      auto p = prev_scope.lock();
      return p->qualifiedLookup(name);
    }
    // there is no scope matching the name,
    // and there are no larger scopes to query.
    return {std::move(scope.error())};
  }

  /*
    lookup "a1::...::aN::x" in scope "a0"
  */
  return scope.value().lookup(name.rest_scope());
}

/*
  lookup name while traversing up the scope tree
*/
[[nodiscard]] auto Scope::qualifiedLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  // if name is of the form "x"
  if (!name.isScoped()) {
    // lookup "x" in current scope
    auto found = bindings.lookup(name);
    if (found) {
      // note: this check prevents a module within a module
      // from accessing the outer modules private variables.
      if (found.value().isPrivate()) {
        return Error{Error::NameIsPrivateInScope, Location{}, name.view()};
      }

      return found;
    }

    // since we didn't find "x" in local
    // scope, try and search the prev_scope.
    if (!isGlobal()) {
      auto p = prev_scope.lock();
      return p->qualifiedLookup(name);
    }
    // "x" isn't in scope.
    return {std::move(found.error())};
  }

  // name is of the form "a0::...::aN::x"
  return qualifiedScopeLookup(name);
}
} // namespace mint
