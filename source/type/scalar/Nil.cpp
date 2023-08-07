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
#include "type/scalar/Nil.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace type {
Nil::Nil() noexcept : Type{Type::Kind::Nil} {}

auto Nil::classof(Ptr type) noexcept -> bool {
  return Type::Kind::Nil == type->kind();
}

[[nodiscard]] bool Nil::equals(Ptr right) const noexcept {
  return llvm::dyn_cast<const Nil>(right) != nullptr;
}

void Nil::print(std::ostream &out) const noexcept { out << "Nil"; }

[[nodiscard]] llvm::Type *Nil::toLLVMImpl(Environment &env) const noexcept {
  return cachedType(env.getLLVMNilType());
}
} // namespace type
} // namespace mint