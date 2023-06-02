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
#include <optional>
#include <unordered_map>

#include "ast/Ast.hpp"

namespace mint {

class Bindings {
public:
  using Key = Identifier;
  using Value = std::pair<Type::Pointer, Ast::Pointer>;
  using Table = std::unordered_map<Key, Value>;
  using iterator = typename Table::iterator;

  class Binding {
  private:
    iterator binding;

  public:
    Binding(iterator binding) noexcept : binding(binding) {}

    [[nodiscard]] auto name() const noexcept -> const Key & {
      return binding->first;
    }
    [[nodiscard]] auto type() const noexcept -> Type::Pointer {
      return binding->second.first;
    }
    [[nodiscard]] auto value() const noexcept -> Ast::Pointer {
      return binding->second.second;
    }
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

  auto bind(Key key, Type::Pointer type, Ast::Pointer value) noexcept
      -> Binding {
    // use insert or assign to allow the caller to update
    // the values being kept track of within the table.
    auto pair = table.insert_or_assign(key, Value{type, value});
    return pair.first;
  }

  [[nodiscard]] auto lookup(Key key) noexcept -> std::optional<Binding> {
    auto found = table.find(key);
    if (found == table.end()) {
      return std::nullopt;
    }
    return {found};
  }
};

class Scope;

class ScopeTable {
public:
  using Key = Identifier;
  using Value = std::shared_ptr<Scope>;
  using Table = std::unordered_map<Key, Value>;

  class Entry {
    Table::iterator iter;

  public:
    Entry(Table::iterator iter) noexcept : iter(iter) {}

    [[nodiscard]] auto empty() const noexcept -> bool;

    auto bind(Identifier name, Type::Pointer type, Ast::Pointer value) noexcept
        -> Bindings::Binding;

    [[nodiscard]] auto lookup(Identifier name) noexcept
        -> std::optional<Bindings::Binding>;
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

  auto emplace(Identifier name) noexcept -> Entry;

  [[nodiscard]] auto lookup(Identifier name) noexcept -> std::optional<Entry> {
    auto found = table.find(name);
    if (found == table.end()) {
      return std::nullopt;
    }
    return found;
  }
};

/*
  a scope represents global scope,
  and any scope within global scope.

  a scope is-a map of identifiers to bindings.
  so a qualified name like "a.b" is resolved
  by looking up "b" within scope "a"

  if an identifier begins with '.' such as '.a'
  this implies lookup starting from the global
  scope.

  what about "a.b.c.d.etc."?
  well the natural way of resolving that would be
  to allow scopes to be bound to names. then we
  can treat each name appearing before a '.' to
  name a scope, which is looked up, and then we
  can delegate lookup to that scope, of the rest
  of the name.
*/

class Scope : public std::enable_shared_from_this<Scope> {
public:
private:
  std::optional<Identifier> name;
  std::weak_ptr<Scope> parent;
  std::weak_ptr<Scope> global;
  Bindings bindings;
  ScopeTable scopes;

  Scope() noexcept = default;
  Scope(Identifier name, std::weak_ptr<Scope> parent) noexcept
      : name(name), parent(parent) {
    auto ptr = parent.lock();
    global = ptr->global;
  }

public:
  [[nodiscard]] auto createGlobalScope() -> std::shared_ptr<Scope> {
    return std::shared_ptr<Scope>(new Scope());
  }

  [[nodiscard]] auto createChildScope(Identifier name,
                                      std::weak_ptr<Scope> parent)
      -> std::shared_ptr<Scope> {
    return std::shared_ptr<Scope>(new Scope(name, parent));
  }

  auto bindName(Identifier name, Type::Pointer type, Ast::Pointer value)
      -> Bindings::Binding {
    return bindings.bind(name, type, value);
  }

  auto bindScope(Identifier name) -> ScopeTable::Entry {
    return scopes.emplace(name);
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

    both scope lookup, and variable lookup first search
    the local scope, and if the variable is not found,
    lookup one scope higher. if we are at global scope
    and we fail to find the binding then lookup fails.
  */
  auto lookup(Identifier name) -> std::optional<Bindings::Binding> { 
  }
};

} // namespace mint
