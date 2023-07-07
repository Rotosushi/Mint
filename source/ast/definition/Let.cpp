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
#include <sstream>

#include "adt/Environment.hpp"
#include "ast/definition/Let.hpp"

namespace mint {
namespace ast {

auto Let::handleUseBeforeDef(Error &error,
                             [[maybe_unused]] Environment &env) const noexcept
    -> Result<type::Ptr> {
  if (error.kind() == Error::Kind::UseBeforeDef) {
  }
  return error;
}

Result<type::Ptr> Let::typecheck(Environment &env) const noexcept {
  auto found = env.lookupBinding(name());
  if (found) {
    return {Error::Kind::NameAlreadyBoundInScope, location(), name().view()};
  }

  /*
    #NOTE: if this let expression is use-before-def,
    then the error is returned here.
  */
  auto term_type_result = m_ast->typecheck(env);
  if (!term_type_result) {
    return term_type_result;
  }
  auto &type = term_type_result.value();

  auto anno = annotation();
  if (anno.has_value()) {
    auto &annotated_type = anno.value();

    if (!annotated_type->equals(type)) {
      std::stringstream message;
      message << annotated_type << " != " << type;
      return {Error::Kind::LetTypeMismatch, location(), message.view()};
    }
  }

  // #NOTE:
  // since we could type this definition, we construct a
  // partial binding, such that definitions appearing after
  // this one and relying upon this definitions type can be
  // typechecked
  auto bound = env.partialBindName(name(), attributes(), type);
  if (!bound)
    return bound.error();

  // #NOTE:
  // since we could construct a partialBinding, we check
  // if we can partially resolve any use-before-def
  // which rely upon this one.
  auto failed = env.partialResolveUseBeforeDef(env.getQualifiedName(name()));
  if (failed)
    return failed.value();

  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Let::evaluate(Environment &env) noexcept {
  /*
    #RULE #NOTE: we create partial bindings during typechecking,
    and complete them during evaluation. this means we
    expect the name to be bound already in scope.
    thus we assert that the binding exists.
  */
  auto found = env.lookupBinding(name());
  MINT_ASSERT(found);

  auto binding = found.value();

  if (binding.hasValue())
    return {Error::Kind::NameAlreadyBoundInScope, location(), name().view()};

  auto term_value_result = m_ast->evaluate(env);
  if (!term_value_result)
    return term_value_result;
  auto &value = term_value_result.value();

  // #NOTE #RULE
  // we bind to a clone of the value, because otherwise
  // the let expression would introduce a reference.
  // this is not the meaning of let, which introduces a
  // new variable. and as such must model the semantics of
  // a new value.
  auto bound = env.completeNameBinding(binding, value->clone());
  if (!bound) // #TODO: add location context to this error before returning
    return bound.error();

  // #NOTE: we just created a new binding, so we can
  // fully typecheck and evaluate any partial bindings
  // that rely on this definition
  auto failed = env.resolveUseBeforeDef(env.getQualifiedName(name()));
  if (failed)
    return failed.value();

  return env.getNilAst({}, location());
}
} // namespace ast
} // namespace mint
