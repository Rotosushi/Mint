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
#include "type/composite/Lambda.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace type {
Lambda::Lambda(type::Function const *function_type) noexcept
    : Type(Type::Kind::Lambda), m_function_type(function_type) {}

auto Lambda::classof(type::Ptr type) noexcept -> bool {
  return Type::Kind::Lambda == type->kind();
}

[[nodiscard]] auto Lambda::function_type() const noexcept
    -> type::Function const * {
  return m_function_type;
}

[[nodiscard]] bool Lambda::equals(type::Ptr type) const noexcept {
  if (auto lambda_type = llvm::dyn_cast<Lambda>(type); lambda_type != nullptr) {
    return m_function_type->equals(lambda_type->m_function_type);
  }
  return false;
}

void Lambda::print(std::ostream &out) const noexcept {
  // #NOTE: do we want to allow the user to spell
  // lambda types somehow? or are they going to be
  // equivalent to function types from the programmers
  // perspective?
  m_function_type->print(out);
}

[[nodiscard]] llvm::Type *Lambda::toLLVMImpl(Environment &env) const noexcept {
  return env.getLLVMPointerType();
}
} // namespace type
} // namespace mint
