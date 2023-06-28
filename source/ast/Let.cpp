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
#include "ast/Let.hpp"

namespace mint {
Result<Type::Ptr> LetAst::typecheck(Environment &env) const noexcept {
  auto term_type_result = m_term->typecheck(env);
  if (!term_type_result)
    return term_type_result;
  auto type = term_type_result.value();

  auto anno = annotation();
  if (anno.has_value()) {
    auto &annotated_type = anno.value();

    if (!annotated_type->equals(type)) {
      std::stringstream message;
      message << annotated_type << " != " << type;
      return {Error::LetTypeMismatch, location(), message.view()};
    }
  }

  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<Ast::Ptr> LetAst::evaluate(Environment &env) noexcept {
  auto term_value_result = m_term->evaluate(env);
  if (!term_value_result)
    return term_value_result;
  auto &value = term_value_result.value();

  auto type = m_term->cachedTypeOrAssert();

  auto bound = env.bindName(name(), attributes(), type, value);
  if (!bound)
    return bound.error();

  // return env.getNilAst({}, location());
}
} // namespace mint
