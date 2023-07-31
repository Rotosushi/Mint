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
#include "adt/Environment.hpp"
#include "ast/All.hpp"
#include "utility/VectorHelpers.hpp"

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
  return iter->second->lookupBinding(name);
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

// return the qualified name of this scope
[[nodiscard]] auto Scope::qualifiedName() const noexcept -> Identifier {
  // if this is global scope, then m_name = ""
  // and we can return that immediately.
  if (isGlobal())
    return name();

  auto prev = m_prev_scope.lock();
  // if this scope is anonymous, we return the qualified name of the
  // enclosing scope.
  if (!hasName()) {
    return prev->qualifiedName();
  }

  // return the qualified name of the previous scope prepended
  // to the name of this scope.
  auto base = name();
  auto result = base.prependScope(prev->qualifiedName());
  return result;
}

// #NOTE: build up the qualified name as we
// walk up to global scope.
[[nodiscard]] auto Scope::qualifyName(Identifier name) noexcept -> Identifier {
  if (isGlobal()) {
    return name.prependScope(this->name());
  }

  // prepend this scopes name, handling the case
  // where the local scope is anonymous
  Identifier base =
      m_name.has_value() ? name.prependScope(m_name.value()) : name;

  // qualify the name with the previous scope
  auto prev = m_prev_scope.lock();
  return prev->qualifyName(base);
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
  auto found = m_scopes->lookup(first);
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
  if (!name.isQualified()) {
    // lookup "x" in current scope
    auto found = m_bindings->lookup(name);
    if (found) {
      if (found.value().isPrivate()) {
        return {
            Error{Error::Kind::NameIsPrivateInScope, Location{}, name.view()}};
      }

      return found;
    }

    // "x" isn't in scope.
    return {std::move(found.error())};
  }

  // name is of the form "a0::...::aN::x"
  return qualifiedScopeLookup(name);
}

auto Scope::lookupBinding(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  if (name.isGloballyQualified()) {
    auto global_scope = m_global.lock();
    return global_scope->qualifiedLookup(name.restScope());
  }

  auto found_locally =
      name.isQualified() ? qualifiedLookup(name) : lookupLocalBinding(name);
  // if we found the name return the binding
  if (found_locally)
    return found_locally;
  // if we cannot search a higher scope return
  // the name unfound error.
  if (isGlobal())
    return found_locally;
  // search the higher scope for the name.
  auto scope = m_prev_scope.lock();
  return scope->qualifiedLookup(name);
}

} // namespace mint
