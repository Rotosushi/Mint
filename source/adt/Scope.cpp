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
[[nodiscard]] auto Bindings::Binding::key() const noexcept -> Key const & {
  return (*this)->first;
}
[[nodiscard]] auto Bindings::Binding::value() const noexcept -> Value const & {
  return (*this)->second;
}
[[nodiscard]] auto Bindings::Binding::value() noexcept -> Value & {
  return (*this)->second;
}
[[nodiscard]] auto Bindings::Binding::attributes() const noexcept
    -> Attributes const & {
  return std::get<Attributes>(value());
}
[[nodiscard]] auto Bindings::Binding::isPublic() const noexcept -> bool {
  return attributes().isPublic();
}
[[nodiscard]] auto Bindings::Binding::isPrivate() const noexcept -> bool {
  return attributes().isPrivate();
}
[[nodiscard]] auto Bindings::Binding::type() const noexcept -> type::Ptr {
  return std::get<type::Ptr>(value());
}

[[nodiscard]] auto Bindings::Binding::comptimeValue() const noexcept
    -> std::optional<ast::Ptr> const & {
  return std::get<std::optional<ast::Ptr>>(value());
}
[[nodiscard]] auto Bindings::Binding::comptimeValue() noexcept
    -> std::optional<ast::Ptr> & {
  return std::get<std::optional<ast::Ptr>>(value());
}
[[nodiscard]] auto Bindings::Binding::hasComptimeValue() const noexcept
    -> bool {
  return comptimeValue().has_value();
}
[[nodiscard]] auto Bindings::Binding::comptimeValueOrAssert() const noexcept
    -> ast::Ptr const & {
  MINT_ASSERT(hasComptimeValue());
  return comptimeValue().value();
}
void Bindings::Binding::setComptimeValue(ast::Ptr ast) noexcept {
  comptimeValue() = std::move(ast);
}

[[nodiscard]] auto Bindings::Binding::runtimeValue() noexcept
    -> std::optional<llvm::Value *> & {
  return std::get<std::optional<llvm::Value *>>(value());
}
[[nodiscard]] auto Bindings::Binding::runtimeValue() const noexcept
    -> std::optional<llvm::Value *> const & {
  return std::get<std::optional<llvm::Value *>>(value());
}
[[nodiscard]] auto Bindings::Binding::hasRuntimeValue() const noexcept -> bool {
  return runtimeValue().has_value();
}
[[nodiscard]] auto Bindings::Binding::runtimeValueOrAssert() const noexcept
    -> llvm::Value * {
  MINT_ASSERT(hasRuntimeValue());
  return runtimeValue().value();
}
void Bindings::Binding::setRuntimeValue(llvm::Value *value) noexcept {
  runtimeValue() = value;
}

[[nodiscard]] auto Bindings::empty() const noexcept -> bool {
  return table.empty();
}

void Bindings::unbind(Key name) noexcept { table.erase(name); }

auto Bindings::bind(Key key, Attributes attributes, type::Ptr type,
                    ast::Ptr comptime_value,
                    llvm::Value *runtime_value) noexcept -> Result<Binding> {
  auto found = lookup(key);
  if (found) {
    return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};
  }
  auto pair = table.try_emplace(key, attributes, type,
                                std::move(comptime_value), runtime_value);
  return {Binding{pair.first}};
}

auto Bindings::partialBind(Key key, Attributes attributes,
                           type::Ptr type) noexcept -> Result<Binding> {
  if (auto found = lookup(key))
    return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};

  auto pair = table.try_emplace(
      key, Value{attributes, type, std::nullopt, std::nullopt});
  return {Binding{pair.first}};
}

[[nodiscard]] auto Bindings::lookup(Key key) noexcept -> Result<Binding> {
  auto found = table.find(key);
  if (found == table.end()) {
    return {Error::Kind::NameUnboundInScope, {}, key.view()};
  }
  return {found};
}

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

[[nodiscard]] auto ScopeTable::Entry::lookupBinding(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->lookupBinding(name);
}

[[nodiscard]] auto ScopeTable::Entry::qualifiedLookup(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return iter->second->qualifiedLookup(name);
}

[[nodiscard]] auto ScopeTable::empty() const noexcept -> bool {
  return table.empty();
}
[[nodiscard]] auto ScopeTable::begin() noexcept -> ScopeTable::iterator {
  return table.begin();
}
[[nodiscard]] auto ScopeTable::end() noexcept -> ScopeTable::iterator {
  return table.end();
}

auto ScopeTable::emplace(Identifier name, Scope *prev_scope) noexcept -> Entry {
  auto pair = table.try_emplace(name, Scope::createScope(name, prev_scope));
  return pair.first;
}

void ScopeTable::unbind(Identifier name) noexcept {
  auto found = table.find(name);
  if (found != table.end()) {
    table.erase(found);
  }
}

[[nodiscard]] auto ScopeTable::lookup(Identifier name) noexcept
    -> Result<Entry> {
  auto found = table.find(name);
  if (found == table.end()) {
    return {Error{Error::Kind::NameUnboundInScope, Location{}, name.view()}};
  }
  return {Entry{found}};
}

Scope::Scope(Identifier name) noexcept
    : m_name(name), m_bindings(std::make_unique<Bindings>()),
      m_scopes(std::make_unique<ScopeTable>()) {}

Scope::Scope(std::optional<Identifier> name, Scope *prev_scope) noexcept
    : m_name(name), m_prev_scope(prev_scope),
      m_bindings(std::make_unique<Bindings>()),
      m_scopes(std::make_unique<ScopeTable>()) {
  m_global = prev_scope->m_global;
}

Scope::Scope(Identifier name, Scope *prev_scope) noexcept
    : m_name(name), m_prev_scope(prev_scope),
      m_bindings(std::make_unique<Bindings>()),
      m_scopes(std::make_unique<ScopeTable>()) {
  m_global = prev_scope->m_global;
}

void Scope::setPrevScope(Scope *scope) noexcept { m_prev_scope = scope; }
void Scope::setGlobal(Scope *scope) noexcept { m_global = scope; }

[[nodiscard]] auto Scope::createGlobalScope(Identifier name)
    -> std::shared_ptr<Scope> {
  MINT_ASSERT(name.view() == "");
  auto scope = std::make_shared<Scope>(name);
  scope->setPrevScope(nullptr);
  scope->setGlobal(scope.get());
  return scope;
}

[[nodiscard]] auto Scope::createScope(std::optional<Identifier> name,
                                      Scope *prev_scope)
    -> std::shared_ptr<Scope> {
  return std::make_shared<Scope>(name, prev_scope);
}

std::shared_ptr<Scope> Scope::pushScope() noexcept {
  auto scope = createScope({}, this);
  m_next_scope = scope;
  return scope;
}

std::shared_ptr<Scope> Scope::pushScope(Identifier name) noexcept {
  auto found = m_scopes->lookup(name);
  if (found) {
    m_next_scope = found.value().ptr();
  } else {
    m_next_scope = m_scopes->emplace(name, this).ptr();
  }

  return m_next_scope;
}

std::shared_ptr<Scope> Scope::popScope() noexcept {
  m_next_scope.reset();
  if (isGlobal()) {
    return shared_from_this();
  }

  return prevScope();
}

[[nodiscard]] auto Scope::isGlobal() const noexcept -> bool {
  return m_prev_scope == nullptr;
}

[[nodiscard]] auto Scope::prevScope() const noexcept -> std::shared_ptr<Scope> {
  // #QUESTION is asserting the precondition the best solution?
  // I like it better than returning a nullptr.
  MINT_ASSERT(nullptr != m_prev_scope);
  return m_prev_scope->shared_from_this();
}

[[nodiscard]] auto Scope::bindingsEmpty() const noexcept -> bool {
  return m_bindings->empty();
}

[[nodiscard]] auto Scope::scopesEmpty() const noexcept -> bool {
  return m_scopes->empty();
}

[[nodiscard]] auto Scope::hasName() const noexcept -> bool {
  return m_name.has_value();
}

[[nodiscard]] auto Scope::name() const noexcept -> Identifier {
  MINT_ASSERT(hasName());
  return m_name.value();
}

// return the qualified name of this scope
[[nodiscard]] auto Scope::qualifiedName() const noexcept -> Identifier {
  // if this is global scope, then m_name = ""
  // and we can return that immediately.
  if (isGlobal())
    return name();
  // if this scope is anonymous, we return the qualified name of the
  // enclosing scope.
  if (!hasName()) {
    return m_prev_scope->qualifiedName();
  }

  // return the qualified name of the previous scope prepended
  // to the name of this scope.
  auto base = name();
  auto prev_name = m_prev_scope->qualifiedName();
  auto result = prev_name.empty() ? base : base.prependScope(prev_name);
  return result;
}

// #NOTE: build up the qualified name as we
// walk up to global scope.
[[nodiscard]] auto Scope::qualifyName(Identifier name) noexcept -> Identifier {
  if (isGlobal()) {
    return name;
  }

  // prepend this scopes name, handling the case
  // where the local scope is anonymous
  Identifier base =
      m_name.has_value() ? name.prependScope(m_name.value()) : name;

  // qualify the name with the previous scope
  return m_prev_scope->qualifyName(base);
}

auto Scope::bindName(Identifier name, Attributes attributes, type::Ptr type,
                     ast::Ptr comptime_value,
                     llvm::Value *runtime_value) noexcept
    -> Result<Bindings::Binding> {
  return m_bindings->bind(name, attributes, type, std::move(comptime_value),
                          runtime_value);
}

auto Scope::partialBindName(Identifier name, Attributes attributes,
                            type::Ptr type) noexcept
    -> Result<Bindings::Binding> {
  return m_bindings->partialBind(name, attributes, type);
}

auto Scope::bindScope(Identifier name) -> ScopeTable::Entry {
  return m_scopes->emplace(name, this);
}

void Scope::unbindScope(Identifier name) { return m_scopes->unbind(name); }

[[nodiscard]] auto Scope::lookupScope(Identifier name) noexcept
    -> Result<ScopeTable::Entry> {
  return m_scopes->lookup(name);
}

[[nodiscard]] auto Scope::lookupLocalBinding(Identifier name) noexcept
    -> Result<Bindings::Binding> {
  return m_bindings->lookup(name);
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
    return m_global->qualifiedLookup(name.restScope());
  }

  // #NOTE: even though qualified lookup also checks name.isQualified
  // we still have to branch here to allow local names to resolve to
  // private variables.
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
  return m_prev_scope->lookupBinding(name);
}

} // namespace mint
