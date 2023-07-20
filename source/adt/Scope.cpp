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
                             type::Ptr type, ast::Ptr comptime_value,
                             llvm::Value *runtime_value) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->bindName(name, attributes, type,
                                std::move(comptime_value), runtime_value);
}

auto ScopeTable::Entry::partialBind(Identifier name, Attributes attributes,
                                    type::Ptr type) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->partialBindName(name, attributes, type);
}

[[nodiscard]] auto ScopeTable::Entry::lookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->lookup(name);
}

[[nodiscard]] auto ScopeTable::Entry::qualifiedLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->qualifiedLookup(name);
}

auto ScopeTable::emplace(Identifier name,
                         std::weak_ptr<Scope> prev_scope) noexcept -> Entry {
  auto pair = table.try_emplace(name, Scope::createScope(name, prev_scope));
  return pair.first;
}

/*
  lookup scope "a0" from name of the form "a0::...::aN::x"
*/
[[nodiscard]] auto Scope::qualifiedScopeLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  /*
    "a0::...::aN::x" -> "a0"
  */
  auto first = name.firstScope();
  auto found = scopes->lookup(first);
  if (!found) {
    return {std::move(found.error())};
  }
  auto &scope = found.value();

  /*
    lookup "a1::...::aN::x" in scope "a0"
  */
  return scope.qualifiedLookup(name.restScope());
}

/*
  lookup name while traversing down the scope tree
*/
[[nodiscard]] auto Scope::qualifiedLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  // if name is of the form "x"
  if (!name.isScoped()) {
    // lookup "x" in current scope
    auto found = bindings->lookup(name);
    if (found) {
      if (found.value().isPrivate()) {
        return Error{Error::Kind::NameIsPrivateInScope, Location{},
                     name.view()};
      }

      return found;
    }

    // "x" isn't in scope.
    return {std::move(found.error())};
  }

  // name is of the form "a0::...::aN::x"
  return qualifiedScopeLookup(name);
}

// #NOTE: walk up to global scope, building up the
// qualified name as we return to the local scope.
[[nodiscard]] auto Scope::getQualifiedNameImpl(Identifier name) noexcept
    -> Identifier {
  if (isGlobal()) {
    auto qualified = name.prependScope(name.globalNamespace());
    return qualified;
  }

  Identifier base = [&]() {
    if (this->name.has_value()) {
      return name.prependScope(this->name.value());
    } else {
      return name;
    }
  }();

  // qualify the name with the previous scope
  auto prev = prev_scope.lock();
  return prev->getQualifiedNameImpl(base);
}

} // namespace mint
