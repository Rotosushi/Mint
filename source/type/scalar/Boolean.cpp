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
#include "type/scalar/Boolean.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace type {
Boolean::Boolean() noexcept : Type{Type::Kind::Boolean} {}

auto Boolean::classof(Ptr type) noexcept -> bool {
  return Type::Kind::Boolean == type->kind();
}

[[nodiscard]] bool Boolean::equals(Ptr right) const noexcept {
  return llvm::dyn_cast<const Boolean>(right) != nullptr;
}

void Boolean::print(std::ostream &out) const noexcept { out << "Boolean"; }

[[nodiscard]] llvm::Type *Boolean::toLLVMImpl(Environment &env) const noexcept {
  return cachedType(env.getLLVMBooleanType());
}
} // namespace type
} // namespace mint
