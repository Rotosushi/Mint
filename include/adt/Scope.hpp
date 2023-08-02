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

    [[nodiscard]] auto key() const noexcept -> Key const &;
    [[nodiscard]] auto value() const noexcept -> Value const &;
    [[nodiscard]] auto value() noexcept -> Value &;

    [[nodiscard]] auto attributes() const noexcept -> Attributes const &;
    [[nodiscard]] auto isPublic() const noexcept -> bool;
    [[nodiscard]] auto isPrivate() const noexcept -> bool;

    [[nodiscard]] auto type() const noexcept -> type::Ptr;

    [[nodiscard]] auto comptimeValue() const noexcept
        -> std::optional<ast::Ptr> const &;
    [[nodiscard]] auto comptimeValue() noexcept -> std::optional<ast::Ptr> &;
    [[nodiscard]] auto hasComptimeValue() const noexcept -> bool;
    [[nodiscard]] auto comptimeValueOrAssert() const noexcept
        -> ast::Ptr const &;
    void setComptimeValue(ast::Ptr ast) noexcept;

    [[nodiscard]] auto runtimeValue() noexcept
        -> std::optional<llvm::Value *> &;
    [[nodiscard]] auto runtimeValue() const noexcept
        -> std::optional<llvm::Value *> const &;
    [[nodiscard]] auto hasRuntimeValue() const noexcept -> bool;
    [[nodiscard]] auto runtimeValueOrAssert() const noexcept -> llvm::Value *;
    void setRuntimeValue(llvm::Value *value) noexcept;
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool;

  void unbind(Key name) noexcept;
  auto bind(Key key, Attributes attributes, type::Ptr type,
            ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept
      -> Result<Binding>;
  auto partialBind(Key key, Attributes attributes, type::Ptr type) noexcept
      -> Result<Binding>;

  [[nodiscard]] auto lookup(Key key) noexcept -> Result<Binding>;
};

class Scope;

class ScopeTable {
public:
  using Key = Identifier;
  using Value = std::shared_ptr<Scope>;
  using Table = std::map<Key, Value>;
  using iterator = Table::iterator;

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
  [[nodiscard]] auto empty() const noexcept -> bool;
  [[nodiscard]] auto begin() noexcept -> iterator;
  [[nodiscard]] auto end() noexcept -> iterator;

  auto emplace(Identifier name, std::weak_ptr<Scope> prev_scope) noexcept
      -> Entry;
  void unbind(Identifier name) noexcept;

  [[nodiscard]] auto lookup(Identifier name) noexcept -> Result<Entry>;
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
  Scope(Identifier name) noexcept;
  Scope(std::optional<Identifier> name,
        std::weak_ptr<Scope> prev_scope) noexcept;
  Scope(Identifier name, std::weak_ptr<Scope> prev_scope) noexcept;

  friend class ScopeTable;

private:
  [[nodiscard]] auto qualifiedScopeLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;
  [[nodiscard]] auto qualifiedLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;

  void setGlobal(std::weak_ptr<Scope> scope) noexcept;

public:
  // #NOTE: global scope has the name "", and this must be
  // provided to this method because Identifiers are interned,
  // so there is no way of statically constructing one.
  [[nodiscard]] static auto createGlobalScope(Identifier name)
      -> std::shared_ptr<Scope>;
  [[nodiscard]] static auto createScope(std::optional<Identifier> name,
                                        std::weak_ptr<Scope> prev_scope)
      -> std::shared_ptr<Scope>;

  [[nodiscard]] auto isGlobal() const noexcept -> bool;
  [[nodiscard]] auto getPrevScope() const noexcept -> std::shared_ptr<Scope>;
  [[nodiscard]] auto hasName() const noexcept;
  // #TODO: if there is no scope name, and this is not global scope
  // walk up the scope tree until we find a scope name. That name is
  // the name of the local named scope. (anonymous scopes are not
  // 'real' scopes, in the sense that they can be named. I think.)
  [[nodiscard]] auto name() const noexcept;
  [[nodiscard]] auto qualifiedName() const noexcept -> Identifier;

  [[nodiscard]] auto bindingsEmpty() const noexcept -> bool {
    return m_bindings->empty();
  }

  [[nodiscard]] auto scopesEmpty() const noexcept -> bool {
    return m_scopes->empty();
  }

  [[nodiscard]] auto qualifyName(Identifier name) noexcept -> Identifier;

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept
      -> Result<Bindings::Binding>;
  auto partialBindName(Identifier name, Attributes attributes,
                       type::Ptr type) noexcept -> Result<Bindings::Binding>;

  auto bindScope(Identifier name) -> ScopeTable::Entry;
  void unbindScope(Identifier name);

  [[nodiscard]] auto lookupScope(Identifier name) noexcept
      -> Result<ScopeTable::Entry>;
  [[nodiscard]] auto lookupLocalBinding(Identifier name) noexcept
      -> Result<Bindings::Binding>;

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
