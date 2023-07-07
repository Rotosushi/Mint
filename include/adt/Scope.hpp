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
#include "adt/VectorMap.hpp"
#include "ast/Ast.hpp"
#include "error/Result.hpp"
/*
  #TODO:
    ) implement a field in bindings to
      allow for public/private visibility.
    ) refactor lookup to return Result<T>
      and construct errors within lookup,
      such that we can distinguish between
      lookup failing due to visibility vs
      name existance.
    ) lookup can only return a private variable
      from it's local scope.
*/

namespace mint {
/* Bindings are a map from Identifiers to (Attributes, Type::Pointer,
  Ast::Pointer) with a more convenient interface.
*/
class Bindings {
public:
  using Key = Identifier;
  using Value = std::tuple<Attributes, type::Ptr, std::optional<ast::Ptr>>;
  using Table = VectorMap<Key, Value>;
  using iterator = typename Table::iterator;

  class Binding : public iterator {
  public:
    Binding(iterator binding) noexcept : iterator(binding) {}

    [[nodiscard]] auto name() const noexcept -> const Key & {
      return (*this)->first;
    }
    [[nodiscard]] auto attributes() const noexcept -> Attributes & {
      return std::get<0>((*this)->second);
    }
    [[nodiscard]] auto isPublic() const noexcept -> bool {
      return attributes().isPublic();
    }
    [[nodiscard]] auto isPrivate() const noexcept -> bool {
      return attributes().isPrivate();
    }
    [[nodiscard]] auto type() const noexcept -> type::Ptr {
      return std::get<1>((*this)->second);
    }
    [[nodiscard]] auto hasValue() const noexcept -> bool {
      return optionalValue().has_value();
    }
    [[nodiscard]] auto optionalValue() const noexcept
        -> std::optional<ast::Ptr> & {
      return std::get<2>((*this)->second);
    }
    [[nodiscard]] auto value() const noexcept -> ast::Ptr {
      MINT_ASSERT(hasValue());
      return optionalValue().value();
    }
    void setValue(ast::Ptr &ast) noexcept { optionalValue() = ast; }
    [[nodiscard]] auto isPartial() const noexcept -> bool {
      return !optionalValue();
    }
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

  void unbind(Key name) noexcept { table.erase(name); }

  auto bind(Key key, Attributes attributes, type::Ptr type,
            ast::Ptr value) noexcept -> Result<Binding> {
    auto found = lookup(key);
    if (found) {
      return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};
    }
    auto pair = table.try_emplace(key, Value{attributes, type, value});
    return Binding{pair.first};
  }

  auto partialBind(Key key, Attributes attributes, type::Ptr type) noexcept
      -> Result<Binding> {
    if (auto found = lookup(key))
      return {Error::Kind::NameAlreadyBoundInScope, {}, key.view()};

    auto pair = table.try_emplace(key, Value{attributes, type, std::nullopt});
    return Binding{pair.first};
  }

  auto completeBinding(Binding binding, ast::Ptr ast) -> Result<Binding> {
    if (binding.hasValue())
      return {Error::Kind::NameAlreadyBoundInScope, {}, binding.name().view()};

    binding.setValue(ast);
    return binding;
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
              ast::Ptr value) noexcept -> Result<Bindings::Binding>;

    [[nodiscard]] auto lookup(Identifier name) noexcept
        -> Result<Bindings::Binding>;
  };

private:
  Table table;

public:
  [[nodiscard]] auto empty() const noexcept -> bool { return table.empty(); }

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
      return Error{Error::Kind::NameUnboundInScope, Location{}, name.view()};
    }
    return Entry{found};
  }
};

class Scope : public std::enable_shared_from_this<Scope> {
private:
  std::optional<Identifier> name;
  std::weak_ptr<Scope> prev_scope;
  std::weak_ptr<Scope> global;
  Bindings bindings;
  ScopeTable scopes;

public:
  Scope() noexcept = default;
  Scope(std::optional<Identifier> name,
        std::weak_ptr<Scope> prev_scope) noexcept
      : name(name), prev_scope(prev_scope) {}
  Scope(Identifier name, std::weak_ptr<Scope> prev_scope) noexcept
      : name(std::move(name)), prev_scope(prev_scope) {
    auto ptr = prev_scope.lock();
    global = ptr->global;
  }

private:
  [[nodiscard]] auto qualifiedScopeLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;
  [[nodiscard]] auto qualifiedLookup(Identifier name) noexcept
      -> Result<Bindings::Binding>;

  [[nodiscard]] auto getQualifiedNameImpl(Identifier name) noexcept
      -> Identifier;

  void setGlobal(std::weak_ptr<Scope> scope) noexcept { global = scope; }

public:
  [[nodiscard]] static auto createGlobalScope() -> std::shared_ptr<Scope> {
    auto global = std::make_shared<Scope>();
    global->setGlobal(global->weak_from_this());
    return global;
  }

  [[nodiscard]] static auto createScope(std::optional<Identifier> name,
                                        std::weak_ptr<Scope> prev_scope)
      -> std::shared_ptr<Scope> {
    return std::make_shared<Scope>(name, prev_scope);
  }

  [[nodiscard]] auto isGlobal() const noexcept -> bool {
    return prev_scope.expired();
  }

  [[nodiscard]] auto getPrevScope() const noexcept -> std::shared_ptr<Scope> {
    // #QUESTION is asserting the precondition the best solution?
    // I like it better than returning a nullptr.
    MINT_ASSERT(!prev_scope.expired());
    return prev_scope.lock();
  }

  [[nodiscard]] auto scopeName() const noexcept
      -> std::optional<std::string_view> {
    return name;
  }

  [[nodiscard]] auto bindingsEmpty() const noexcept -> bool {
    return bindings.empty();
  }

  [[nodiscard]] auto scopesEmpty() const noexcept -> bool {
    return scopes.empty();
  }

  [[nodiscard]] auto getQualifiedName(Identifier name) noexcept -> Identifier {
    auto variable = name.variable();
    return getQualifiedNameImpl(variable);
  }

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr value) noexcept {
    return bindings.bind(name, attributes, type, value);
  }

  auto partialBind(Identifier name, Attributes attributes,
                   type::Ptr type) noexcept {
    return bindings.partialBind(name, attributes, type);
  }

  auto completeBinding(Bindings::Binding binding, ast::Ptr ast) noexcept {
    return bindings.completeBinding(binding, ast);
  }

  auto bindScope(Identifier name) {
    return scopes.emplace(name, weak_from_this());
  }

  void unbindScope(Identifier name) { return scopes.unbind(name); }

  [[nodiscard]] auto lookupScope(Identifier name) noexcept {
    return scopes.lookup(name);
  }

  [[nodiscard]] auto lookupLocal(Identifier name) noexcept {
    return bindings.lookup(name);
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
    the local scope, and if the variable/scope is not found,
    lookup one scope higher. if we are at global scope
    and we fail to find the binding then lookup fails.
  */
  [[nodiscard]] auto lookup(Identifier name) noexcept
      -> Result<Bindings::Binding> {
    auto found = lookupLocal(name);
    if (!found) {
      if (isGlobal()) {
        return qualifiedLookup(name);
      }
      auto prev = prev_scope.lock();
      return prev->qualifiedLookup(name);
    }
    return found;
  }
};

} // namespace mint
