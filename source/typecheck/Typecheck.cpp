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
#include "typecheck/Typecheck.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"

namespace mint {
struct TypecheckScalar {
  Environment *env;

  TypecheckScalar(Environment &env) noexcept : env(&env) {}

  Result<type::Ptr> operator()(ir::detail::Scalar &scalar) noexcept {
    return std::visit(*this, scalar.variant());
  }

  Result<type::Ptr> operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return env->getNilType();
  }

  Result<type::Ptr> operator()([[maybe_unused]] bool &boolean) noexcept {
    return env->getBooleanType();
  }

  Result<type::Ptr> operator()([[maybe_unused]] int &integer) noexcept {
    return env->getIntegerType();
  }
};

Result<type::Ptr> typecheck(ir::detail::Scalar &scalar,
                            Environment &env) noexcept {
  TypecheckScalar visitor(env);
  return visitor(scalar);
}

struct TypecheckParameter {
  ir::Mir *ir;
  Environment *env;

  TypecheckParameter(ir::Mir &ir, Environment &env) noexcept
      : env(&env), ir(&ir) {}

  Result<type::Ptr> operator()(ir::detail::Parameter &parameter) noexcept {
    return std::visit(*this, parameter.variant());
  }

  Result<type::Ptr> operator()(ir::detail::Scalar &scalar) noexcept {
    return typecheck(scalar, *env);
  }

  Result<type::Ptr> operator()(Identifier &variable) noexcept {
    auto result = env->lookupBinding(variable);
    if (!result)
      return result.error();
    // #TODO: handle use-before-def
    // #TODO: create a type cache
    return result.value().type();
  }
};

Result<type::Ptr> typecheck(ir::Mir &ir, Environment &env) noexcept {}
} // namespace mint
