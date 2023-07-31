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
#include <map>
#include <optional>
#include <set>
#include <tuple>

#include "adt/Attributes.hpp"
#include "adt/Identifier.hpp"
// #include "adt/UseBeforeDefMap.hpp"
#include "ast/Ast.hpp"
#include "error/Result.hpp"

namespace mint {
class Bindings {
public:
  using Key = Identifier;
  using Value = std::tuple<Attributes, type::Ptr, std::optional<ast::Ptr>,
                           std::optional<llvm::Value *>>;
  using Table = std::map<Key, Value>;
  using iterator = typename Table::iterator;

  class Binding : public Table::iterator {
  public:
    Binding(iterator binding) noexcept : Table::iterator(binding) {}

    [[nodiscard]] auto key() const noexcept -> Key const & {
      return (*this)->first;
    }
    [[nodiscard]] auto value() const noexcept -> Value const & {
      return (*this)->second;
    }
    [[nodiscard]] auto value() noexcept -> Value & { return (*this)->second; }
    [[nodiscard]] auto attributes() const noexcept -> Attributes const & {
      return std::get<Attributes>(value());
    }
    [[nodiscard]] auto isPublic() const noexcept -> bool {
      return attributes().isPublic();
    }
    [[nodiscard]] auto isPrivate() const noexcept -> bool {
      return attributes().isPrivate();
    }
    [[nodiscard]] auto type() const noexcept -> type::Ptr {
      return std::get<type::Ptr>(value());
    }

    [[nodiscard]] auto comptimeValue() const noexcept
        -> std::optional<ast::Ptr> const & {
      return std::get<std::optional<ast::Ptr>>(value());
    }
    [[nodiscard]] auto comptimeValue() noexcept -> std::optional<ast::Ptr> & {
      return std::get<std::optional<ast::Ptr>>(value());
    }
    [[nodiscard]] auto hasComptimeValue() const noexcept -> bool {
      return comptimeValue().has_value();
    }
    [[nodiscard]] auto comptimeValueOrAssert() const noexcept
        -> ast::Ptr const & {
      MINT_ASSERT(hasComptimeValue());
      return comptimeValue().value();
    }
    void setComptimeValue(ast::Ptr ast) noexcept {
      comptimeValue() = std::move(ast);
    }

    [[nodiscard]] auto runtimeValue() noexcept
        -> std::optional<llvm::Value *> & {
      return std::get<std::optional<llvm::Value *>>(value());
    }
    [[nodiscard]] auto runtimeValue() const noexcept
        -> std::optional<llvm::Value *> const & {
      return std::get<std::optional<llvm::Value *>>(value());
    }
    [[nodiscard]] auto hasRuntimeValue() const noexcept -> bool {
      return runtimeValue().has_value();
    }
    [[nodiscard]] auto runtimeValueOrAssert() const noexcept -> llvm::Value * {
      MINT_ASSERT(hasRuntimeValue());
      return runtimeValue().value();
    }
    void setRuntimeValue(llvm::Value *value) noexcept {
      runtimeValue() = value;
    }
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

  void unbind(Key name) noexcept { table.erase(name); }

  auto bind(Key key, Attributes attributes, type::Ptr type,
            ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept
      -> Result<Binding> {
    auto found = lookup(key);
    if (found) {
      return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};
    }
    auto pair = table.try_emplace(key, attributes, type,
                                  std::move(comptime_value), runtime_value);
    return {Binding{pair.first}};
  }

  auto partialBind(Key key, Attributes attributes, type::Ptr type) noexcept
      -> Result<Binding> {
    if (auto found = lookup(key))
      return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};

    auto pair = table.try_emplace(
        key, Value{attributes, type, std::nullopt, std::nullopt});
    return {Binding{pair.first}};
  }

  [[nodiscard]] auto lookup(Key key) noexcept -> Result<Binding> {
    auto found = table.find(key);
    if (found == table.end()) {
      return {Error::Kind::NameUnboundInScope, {}, key.view()};
    }
    return {found};
  }
};

class Scope;

class ScopeTable {
public:
  using Key = Identifier;
  using Value = std::shared_ptr<Scope>;
  using Table = std::map<Key, Value>;

  class Entry {
    Table::iterator iter;

  public:
    Entry(Table::iterator iter) noexcept : iter(iter) {}

    [[nodiscard]] auto ptr() const noexcept -> std::shared_ptr<Scope>;

    [[nodiscard]] auto namesEmpty() const noexcept -> bool;

    [[nodiscard]] auto scopesEmpty() const noexcept -> bool;

    auto bind(Identifier name, Attributes attributes, type::Ptr type,
              ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept
        -> Result<Bindings::Binding>;

    auto partialBind(Identifier name, Attributes attributes,
                     type::Ptr type) noexcept -> Result<Bindings::Binding>;

    [[nodiscard]] auto lookup(Identifier name) noexcept
        -> Result<Bindings::Binding>;

    [[nodiscard]] auto qualifiedLookup(Identifier name) noexcept
        -> Result<Bindings::Binding>;
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }
  [[nodiscard]] auto begin() noexcept { return table.begin(); }
  [[nodiscard]] auto end() noexcept { return table.end(); }

  auto emplace(Identifier name, std::weak_ptr<Scope> prev_scope) noexcept
      -> Entry;

  void unbind(Identifier name) noexcept {
    auto found = table.find(name);
    if (found != table.end()) {
      table.erase(found);
    }
  }

  [[nodiscard]] auto lookup(Identifier name) noexcept -> Result<Entry> {
    auto found = table.find(name);
    if (found == table.end()) {
      return {Error{Error::Kind::NameUnboundInScope, Location{}, name.view()}};
    }
    return {Entry{found}};
  }
};

class Scope : public std::enable_shared_from_this<Scope> {
private:
  std::optional<Identifier> m_name;
  std::weak_ptr<Scope> m_prev_scope;
  std::weak_ptr<Scope> m_global;
  std::unique_ptr<Bindings> m_bindings;
  std::unique_ptr<ScopeTable> m_scopes;
  // UseBeforeDefMap m_use_before_def_map;

public:
  Scope(Identifier name) noexcept
      : m_name(name), m_bindings(std::make_unique<Bindings>()),
        m_scopes(std::make_unique<ScopeTable>()) {}
  Scope(std::optional<Identifier> name,
        std::weak_ptr<Scope> prev_scope) noexcept
      : m_name(name), m_prev_scope(prev_scope),
        m_bindings(std::make_unique<Bindings>()),
        m_scopes(std::make_unique<ScopeTable>()) {
    auto ptr = prev_scope.lock();
    m_global = ptr->m_global;
  }
  Scope(Identifier name, std::weak_ptr<Scope> prev_scope) noexcept
      : m_name(std::move(name)), m_prev_scope(prev_scope),
        m_bindings(std::make_unique<Bindings>()),
        m_scopes(std::make_unique<ScopeTable>()) {
    auto ptr = prev_scope.lock();
    m_global = ptr->m_global;
  }

  friend class ScopeTable;

private:
  [[nodiscard]] auto qualifiedScopeLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;
  [[nodiscard]] auto qualifiedLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;

  void setGlobal(std::weak_ptr<Scope> scope) noexcept { m_global = scope; }

public:
  // #NOTE: global scope has the name "", and this must be
  // provided to this method because Identifiers are interned,
  // so there is no way of statically constructing one.
  [[nodiscard]] static auto createGlobalScope(Identifier name)
      -> std::shared_ptr<Scope> {
    MINT_ASSERT(name.view() == "");
    auto scope = std::make_shared<Scope>(name);
    scope->setGlobal(scope->weak_from_this());
    return scope;
  }

  [[nodiscard]] static auto createScope(std::optional<Identifier> name,
                                        std::weak_ptr<Scope> prev_scope)
      -> std::shared_ptr<Scope> {
    return std::make_shared<Scope>(name, prev_scope);
  }

  [[nodiscard]] auto isGlobal() const noexcept -> bool {
    return m_prev_scope.expired();
  }

  [[nodiscard]] auto getPrevScope() const noexcept -> std::shared_ptr<Scope> {
    // #QUESTION is asserting the precondition the best solution?
    // I like it better than returning a nullptr.
    MINT_ASSERT(!m_prev_scope.expired());
    return m_prev_scope.lock();
  }

  [[nodiscard]] auto hasName() const noexcept { return m_name.has_value(); }
  // #TODO: if there is no scope name, and this is not global scope
  // walk up the scope tree until we find a scope name. That name is
  // the name of the local named scope. (anonymous scopes are not
  // 'real' scopes, in the sense that they can be named. I think.)
  [[nodiscard]] auto name() const noexcept {
    MINT_ASSERT(hasName());
    return m_name.value();
  }
  [[nodiscard]] auto qualifiedName() const noexcept -> Identifier;

  [[nodiscard]] auto bindingsEmpty() const noexcept -> bool {
    return m_bindings->empty();
  }

  [[nodiscard]] auto scopesEmpty() const noexcept -> bool {
    return m_scopes->empty();
  }

  [[nodiscard]] auto qualifyName(Identifier name) noexcept -> Identifier;

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept {
    return m_bindings->bind(name, attributes, type, std::move(comptime_value),
                            runtime_value);
  }

  auto partialBindName(Identifier name, Attributes attributes,
                       type::Ptr type) noexcept {
    return m_bindings->partialBind(name, attributes, type);
  }

  auto bindScope(Identifier name) {
    return m_scopes->emplace(name, weak_from_this());
  }

  void unbindScope(Identifier name) { return m_scopes->unbind(name); }

  [[nodiscard]] auto lookupScope(Identifier name) noexcept {
    return m_scopes->lookup(name);
  }

  [[nodiscard]] auto lookupLocalBinding(Identifier name) noexcept {
    return m_bindings->lookup(name);
  }

  /*
    lookup a name of the form 'a::b::c::d::...::w::x::y::z'

    where 'a::b::c::d::...::w::x::y' are all considered to be
    scopes, due to their being prefixes to a name.
    and 'z' is the name bound to some value, due to it
    being a suffix to some value.

    "a"
    "a" is treated as a local variable

    "::a"
    "a" is treated as a global variable.

    "a::b"
    "a" is considered a scope,
    "b" is considered a variable local to scope "a"

    "a0::a1::...::aN::x"
    "a0,a1,...,aN" are all considered scopes,
    "x" is considered a variable local to scope "aN"
  */
  [[nodiscard]] auto lookupBinding(Identifier name) noexcept
      -> Result<Bindings::Binding>;
};

} // namespace mint
