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
[[nodiscard]] auto ScopeTable::Entry::namesEmpty() const noexcept -> bool {
  return iter->second->namesEmpty();
}

[[nodiscard]] auto ScopeTable::Entry::scopesEmpty() const noexcept -> bool {
  return iter->second->scopesEmpty();
}

auto ScopeTable::Entry::bind(Identifier name, Type::Pointer type,
                             Ast::Pointer value) noexcept -> Bindings::Binding {
  return iter->second->bindName(std::move(name), type, value);
}

[[nodiscard]] auto ScopeTable::Entry::lookup(Identifier name) noexcept
    -> std::optional<Bindings::Binding> {
  return iter->second->lookup(name);
}

auto ScopeTable::emplace(Identifier name,
                         std::weak_ptr<Scope> prev_scope) noexcept -> Entry {
  auto iter = table.emplace();
}

/*
  name is guaranteed to be of the form
  "a0::...::aN::x"
*/
[[nodiscard]] auto Scope::qualifiedLookup(Identifier name) noexcept
    -> std::optional<Bindings::Binding> {
  /*
    if name begins with "::"
  */
  if (name.globallyQualified()) {
    auto g = global.lock();
    return g->lookup(name.variable());
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
      return p->lookup(name);
    }
    // there is no scope matching the name,
    // and there are no larger scopes to query.
    return std::nullopt;
  }

  /*
    lookup "a1::...::aN::x" in scope "a0"
  */
  return scope->lookup(name.rest_scope());
}

[[nodiscard]] auto Scope::lookup(Identifier name) noexcept
    -> std::optional<Bindings::Binding> {
  // if name is of the form "x"
  if (!name.isScoped()) {
    auto found = bindings.lookup(name);
    if (found) {
      return found;
    }

    if (!isGlobal()) {
      auto p = prev_scope.lock();
      return p->lookup(name);
    }
    return std::nullopt;
  }

  return qualifiedLookup(name);
}
} // namespace mint
