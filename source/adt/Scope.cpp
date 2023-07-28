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

// #NOTE: walk up to global scope, building up the
// qualified name as we return to the local scope.
[[nodiscard]] auto Scope::getQualifiedName(Identifier name) noexcept
    -> Identifier {
  if (isGlobal()) {
    return name;
  }

  // prepend this scopes name
  Identifier base =
      m_name.has_value() ? name.prependScope(m_name.value()) : name;

  // qualify the name with the previous scope
  auto prev = m_prev_scope.lock();
  return prev->getQualifiedName(base);
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

[[nodiscard]] auto
Scope::lookupUseBeforeDefAtLocalScope(Identifier undef,
                                      Identifier q_undef) noexcept
    -> std::vector<UseBeforeDefMap::Range> {
  std::vector<UseBeforeDefMap::Range> results;
  // search the local scope for ubds
  {
    auto result = lookupUseBeforeDefWithinThisScope(undef);
    if (result)
      results.push_back(result.value());
  }
  {
    auto result = lookupUseBeforeDefWithinThisScope(q_undef);
    if (result)
      results.push_back(result.value());
  }

  // search the lower scopes for ubds
  for (auto &pair : *m_scopes) {
    auto &scope = pair.second;
    auto result = scope->lookupUseBeforeDefBelowLocalScope(undef, q_undef);
    if (!result.empty())
      append(results, result);
  }

  // search the higher scopes for ubds
  if (!isGlobal()) {
    MINT_ASSERT(m_name.has_value());
    auto local_scope_name = m_name.value();
    auto scope = m_prev_scope.lock();
    auto result =
        scope->lookupUseBeforeDefAboveLocalScope(q_undef, local_scope_name);
    if (!result.empty())
      append(results, result);
  }

  return results;
}

[[nodiscard]] auto
Scope::lookupUseBeforeDefAboveLocalScope(Identifier q_undef,
                                         Identifier prev_scope_name) noexcept
    -> std::vector<UseBeforeDefMap::Range> {
  std::vector<UseBeforeDefMap::Range> results;
  // only search for qualified ubds. as that is the only
  // way to traverse into the scope local to the definition
  // and use it

  // search in the current scope
  auto result = lookupUseBeforeDefWithinThisScope(q_undef);
  if (result)
    results.push_back(result.value());

  // search in each scope within this scope
  for (auto &pair : *m_scopes) {
    auto &scope = pair.second;
    auto &scope_name = pair.first;
    // don't traverse back into the scope we just came from
    if (scope_name != prev_scope_name) {
      auto result = scope->lookupUseBeforeDefAtParallelScope(q_undef);
      if (!result.empty())
        append(results, result);
    }
  }

  if (!isGlobal()) {
    // we don't want to recurse back down into the current
    // scope when we traverse upwards within this algorithm.
    MINT_ASSERT(m_name.has_value());
    auto current_scope_name = m_name.value();
    auto scope = m_prev_scope.lock();
    auto result =
        scope->lookupUseBeforeDefAboveLocalScope(q_undef, current_scope_name);
    if (!result.empty())
      append(results, result);
  }
  return results;
}

[[nodiscard]] auto
Scope::lookupUseBeforeDefAtParallelScope(Identifier q_undef) noexcept
    -> std::vector<UseBeforeDefMap::Range> {
  std::vector<UseBeforeDefMap::Range> results;
  // we are looking within a scope which is sitting to the side of
  // the scope of the definition we are resolving ubds reliant upon.
  // thus, any names defined within this scope must use the
  // qualified name to resolve the definition, and any scopes defined
  // within the current scope are local to the current scope.

  // search in the local scope
  auto result = lookupUseBeforeDefWithinThisScope(q_undef);
  if (result)
    results.push_back(result.value());

  // search in any subscopes
  for (auto &pair : *m_scopes) {
    auto &scope = pair.second;
    auto result = scope->lookupUseBeforeDefAtParallelScope(q_undef);
    if (!result.empty())
      append(results, result);
  }

  // we have to have come from the above scope,
  // so there is no need to search there
  return results;
}

[[nodiscard]] auto
Scope::lookupUseBeforeDefBelowLocalScope(Identifier undef,
                                         Identifier q_undef) noexcept
    -> std::vector<UseBeforeDefMap::Range> {
  std::vector<UseBeforeDefMap::Range> results;
  // search for either qualified or unqualified ubds as
  // either will allow a lower scope to access the local definition
  {
    auto result = lookupUseBeforeDefWithinThisScope(undef);
    if (result)
      results.push_back(result.value());
  }
  {
    auto result = lookupUseBeforeDefWithinThisScope(q_undef);
    if (result)
      results.push_back(result.value());
  }

  // search the lower scopes for any ubds
  for (auto &pair : *m_scopes) {
    auto &scope = pair.second;
    auto result = scope->lookupUseBeforeDefBelowLocalScope(undef, q_undef);
    if (!result.empty())
      append(results, result);
  }
  return results;
}

[[nodiscard]] auto
Scope::lookupUseBeforeDefWithinThisScope(Identifier name) noexcept
    -> std::optional<UseBeforeDefMap::Range> {
  auto result = m_use_before_def_map.lookup(name);
  if (result.begin() == result.end())
    return std::nullopt;
  else
    return result;
}

/*
  search the entire scope tree for all ubds which
  rely on the given name. returning a list of ranges
  for each range of ubds found which rely on the given
  name.
*/
auto Scope::lookupUseBeforeDef(Identifier undef, Identifier q_undef) noexcept
    -> std::vector<UseBeforeDefMap::Range> {
  return lookupUseBeforeDefAtLocalScope(undef, q_undef);
}

std::optional<Error> Scope::bindUseBeforeDef(const Error &error,
                                             ast::Ptr ast) noexcept {
  auto &use_before_def = error.getUseBeforeDef();
  auto &undef = use_before_def.names.undef;
  auto &q_undef = use_before_def.names.qualified_undef;
  auto &def = use_before_def.names.def;
  auto &q_def = use_before_def.names.qualified_def;
  auto &scope = use_before_def.scope;
  // sanity check that the Ast we are currently processing
  // is a Definition itself. as it only makes sense
  // to bind a Definition in the use-before-def-map
  auto def_ast = dynCast<ast::Definition>(ast.get());
  MINT_ASSERT(def_ast != nullptr);
  // sanity check that the name of the Definition is the same
  // as the name returned by the use-before-def Error.
  MINT_ASSERT(def == def_ast->name());
  // sanity check that the use-before-def is not
  // an unresolvable trivial loop.
  // #TODO: this check needs to be made more robust moving forward.
  // let expressions obviously cannot rely upon their
  // own definition, thus they form the trivial loop.
  // however a function definition can.
  // a new type can, but only under certain circumstances.
  if (auto *let_ast = dynCast<ast::Let>(ast.get()); let_ast != nullptr) {
    if (q_undef == q_def) {
      std::stringstream message;
      message << "Definition [" << ast << "] relies upon itself [" << undef
              << "]";
      return Error{Error::Kind::TypeCannotBeResolved, ast->location(),
                   message.view()};
    }
  }
  // create an entry within the use-before-def-map for this
  // use-before-def Error
  m_use_before_def_map.insert(use_before_def.names, std::move(ast), scope);
  return std::nullopt;
}

std::optional<Error>
Scope::resolveTypeOfUseBeforeDef(Identifier def, Identifier q_def,
                                 Environment &env) noexcept {
  auto ubds = lookupUseBeforeDef(def, q_def);
  for (UseBeforeDefMap::Range &range : ubds) {
    auto result = resolveTypeOfUseBeforeDef(range, env);
    if (!result)
      return result;
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<Error>
Scope::resolveTypeOfUseBeforeDef(UseBeforeDefMap::Range &range,
                                 Environment &env) noexcept {
  std::vector<std::tuple<UseBeforeDefNames, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::iterator> old_entries;
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto definition = cursor.def();
    auto ast = cursor.ast().get();
    [[maybe_unused]] auto &def_scope = cursor.scope();
    // save the current scope, and enter the scope
    // that the definition appears in
    auto old_local = env.exchangeLocalScope(def_scope);
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();

    // attempt to typecheck the definition, in the
    // current environment. (where the undef name
    // has just been defined.)
    auto typecheck_result = ast->typecheck(env);

    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (!error.isUseBeforeDef()) {
        env.exchangeLocalScope(old_local); // restore scope
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.names.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.names, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    env.exchangeLocalScope(old_local); // restore scope
    ++cursor;
  }

  // remove any out of date entries in the map
  if (!old_entries.empty())
    for (auto &entry : old_entries)
      m_use_before_def_map.erase(entry);
  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty())
    for (auto &usedef : stage)
      m_use_before_def_map.insert(std::get<0>(usedef),
                                  std::move(std::get<1>(usedef)),
                                  std::get<2>(usedef));

  return std::nullopt;
}

/*
  called when a new name is defined.
  any use-before-def definitions that rely
  upon the name that was just defined are
  attempted to be resolved here.
*/
std::optional<Error>
Scope::resolveComptimeValueOfUseBeforeDef(Identifier def, Identifier q_def,
                                          Environment &env) noexcept {
  auto ubds = lookupUseBeforeDef(def, q_def);
  for (auto &range : ubds) {
    auto error = resolveComptimeValueOfUseBeforeDef(range, env);
    if (error)
      return error;
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<Error>
Scope::resolveComptimeValueOfUseBeforeDef(UseBeforeDefMap::Range &range,
                                          Environment &env) noexcept {
  std::vector<std::tuple<UseBeforeDefNames, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::iterator> old_entries;

  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto def = cursor.def();
    auto ast = cursor.ast().get();
    [[maybe_unused]] auto def_scope = cursor.scope();
    auto old_local = env.exchangeLocalScope(def_scope);
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();

    /*
      when we resolve a use before def, we are now in a situation
      where we have already created a partial binding of the
      use before def term to it's type. via resolveTypeOfUseBeforeDef.
      thus we have already typechecked the term being fully resolved.
      thus all that is needed is to fully resolve the definition.
    */
    // sanity check that we have already called typecheck on
    // this ast and it succeeded.
    [[maybe_unused]] auto type = ast->cachedTypeOrAssert();

    // create the full binding. resolving the use-before-def
    auto evaluate_result = ast->evaluate(env);
    if (!evaluate_result) {
      auto &error = evaluate_result.error();
      if (!error.isUseBeforeDef()) {
        env.exchangeLocalScope(old_local);
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.names.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.names, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    env.exchangeLocalScope(old_local);
    ++cursor;
  }

  // remove the old use before defs
  if (!old_entries.empty())
    for (auto &entry : old_entries)
      m_use_before_def_map.erase(entry);

  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty()) {
    for (auto &usedef : stage)
      m_use_before_def_map.insert(std::get<0>(usedef),
                                  std::move(std::get<1>(usedef)),
                                  std::get<2>(usedef));
  }

  return std::nullopt;
}

std::optional<Error>
Scope::resolveRuntimeValueOfUseBeforeDef(Identifier def, Identifier q_def,
                                         Environment &env) noexcept {
  auto ubds = lookupUseBeforeDef(def, q_def);
  for (auto &range : ubds) {
    auto error = resolveRuntimeValueOfUseBeforeDef(range, env);
    if (error)
      return error;
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<Error>
Scope::resolveRuntimeValueOfUseBeforeDef(UseBeforeDefMap::Range &range,
                                         Environment &env) noexcept {
  std::vector<std::tuple<UseBeforeDefNames, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::iterator> old_entries;

  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto def = cursor.def();
    auto ast = cursor.ast().get();
    [[maybe_unused]] auto def_scope = cursor.scope();
    auto old_local = env.exchangeLocalScope(def_scope);
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();

    /*
      when we resolve a use before def, we are now in a situation
      where we have already created a partial binding of the
      use before def term to it's type. via resolveTypeOfUseBeforeDef.
      thus we have already typechecked the term being fully resolved.
      thus all that is needed is to fully resolve the definition.
    */
    // sanity check that we have already called typecheck on
    // this ast and it succeeded.
    [[maybe_unused]] auto type = ast->cachedTypeOrAssert();

    // create the full binding. resolving the use-before-def
    auto codegen_result = ast->codegen(env);
    if (!codegen_result) {
      auto &error = codegen_result.error();
      if (!error.isUseBeforeDef()) {
        env.exchangeLocalScope(old_local);
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.names.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.names, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    env.exchangeLocalScope(old_local);
    ++cursor;
  }

  // remove the resolved use before defs
  // #NOTE: once the runtime value has
  // been resolved, there is nothing more
  // to resolve for a given binding.
  m_use_before_def_map.erase(range);

  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty()) {
    for (auto &usedef : stage)
      m_use_before_def_map.insert(std::get<0>(usedef),
                                  std::move(std::get<1>(usedef)),
                                  std::get<2>(usedef));
  }

  return std::nullopt;
}

} // namespace mint
