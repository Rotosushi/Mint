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
#include "adt/Environment.hpp"
#include "ast/definition/Definition.hpp"
#include "core/Typecheck.hpp"

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
auto UseBeforeDefMap::iterator::ubd_def_ir() noexcept -> ir::Mir & {
  return (*this)->m_def_ir;
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

    // handle the case where the ubd_name matches the
    // given name directly.
    if (ubd_name == name) {
      result.append(cursor);
      ++cursor;
      continue;
    }

    // handle the case where the ubd_name has
    // global qualifications. in this case,
    // the only way the new definition could
    // resolve to the udb_name is if with global
    // qualifications itself the names matched.
    if (ubd_name.isGloballyQualified()) {
      auto globally_qualified_name =
          name.prependScope(name.globalQualification());
      if (ubd_name == globally_qualified_name) {
        result.append(cursor);
        ++cursor;
        continue;
      }
      // else fallthrough
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
    // since we know the name is qualified,
    // if we assume that the ubd_name is qualified such that
    // it would resolve to the new definition from the scope
    // in which the ubd_definition was defined, all we need to do
    // is search for the composite name where the ubd_name is
    // qualified within the scope of the ubd_definition
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
                             Identifier scope_name, ir::Mir ir, ast::Ptr ast,
                             std::shared_ptr<Scope> scope) noexcept {
  // #NOTE: we allow multiple definitions to be bound to
  // the same undef name, however we want to prevent the
  // same definition being bound in the table under the
  // same undef name twice.
  auto range = lookup(ubd_name);
  if (contains_definition(range, ubd_name, ubd_def_name))
    return;

  elements.emplace(elements.end(), ubd_name, ubd_def_name, scope_name,
                   std::move(ir), std::move(ast), scope, false);
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

std::optional<Error>
UseBeforeDefMap::bindUseBeforeDef(Identifier undef, Identifier def,
                                  std::shared_ptr<Scope> const &scope,
                                  ir::Mir ir) noexcept {
  auto scope_name = scope->qualifiedName();

  if (undef == def) {
    return {Error::Kind::TypeCannotBeResolved};
  }

  insert(undef, def, scope_name, std::move(ir), {}, scope);
  return std::nullopt;
}

std::optional<Error> UseBeforeDefMap::bindUseBeforeDef(Error const &error,
                                                       ast::Ptr ast) noexcept {
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.undef;
  auto ubd_def_name = names.def;
  auto scope_name = scope->qualifiedName();

  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  insert(ubd_name, ubd_def_name, scope_name, {}, std::move(ast), scope);
  return std::nullopt;
}

std::optional<Error> UseBeforeDefMap::bindUseBeforeDef(Elements &elements,
                                                       Error const &error,
                                                       ast::Ptr ast) noexcept {
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.undef;
  auto ubd_def_name = names.def;
  auto scope_name = scope->qualifiedName();

  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  elements.emplace_back(ubd_name, ubd_def_name, scope_name, ir::Mir{},
                        std::move(ast), scope);
  return std::nullopt;
}

std::optional<Error>
UseBeforeDefMap::resolveTypeOfUseBeforeDef(Environment &env,
                                           Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    // #NOTE: we enter the local scope of the ubd definition
    // so we know that when we construct it's binding we
    // construct it in the correct scope.
    auto old_local_scope = env.exchangeLocalScope(it.scope());

    if (it.ubd_def_ast()) {
      auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
      // #NOTE: since we are resolving the ubd here, we can clear the error
      ubd_def_ast->clearUseBeforeDef();

      auto result = ubd_def_ast->typecheck(env);
      if (!result) {
        auto error = result.error();
        if (!error.isUseBeforeDef()) {
          it.being_resolved(false);
          env.exchangeLocalScope(old_local_scope);
          return error;
        }

        // handle another use before def error.
        bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
        old_entries.append(it);
      }
    } else if (it.ubd_def_ir()) {
      auto &ubd_ir = it.ubd_def_ir();

      auto result = typecheck(ubd_ir, env);
      if (!result) {
        if (result.recovered()) {
          // bindUseBeforeDef was called by the resolver
          old_entries.append(it);
          // fallthrough
        } else {
          it.being_resolved(false);
          env.exchangeLocalScope(old_local_scope);
          return result.error();
        }
      }
    } else {
      abort("cannot resolve type of UBD.");
    }

    env.exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    erase(old_entries);

  if (!stage.empty())
    insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error> UseBeforeDefMap::resolveComptimeValueOfUseBeforeDef(
    Environment &env, Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = env.exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->evaluate(env);
    if (!result) {
      auto error = result.error();
      if (!error.isUseBeforeDef()) {
        it.being_resolved(false);
        env.exchangeLocalScope(old_local_scope);
        return error;
      }

      // handle another use before def error.
      bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
      old_entries.append(it);
    }

    env.exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    erase(old_entries);

  if (!stage.empty())
    insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error> UseBeforeDefMap::resolveRuntimeValueOfUseBeforeDef(
    Environment &env, Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;

  auto range = lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = env.exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->codegen(env);
    if (!result) {
      it.being_resolved(false);
      env.exchangeLocalScope(old_local_scope);
      return result.error();
    }

    env.exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  // #NOTE: codegen is the last step when processing an ast.
  // so we can safely remove these entries from the ubd map
  erase(range);

  if (!stage.empty())
    insert(std::move(stage));

  return std::nullopt;
}

} // namespace mint