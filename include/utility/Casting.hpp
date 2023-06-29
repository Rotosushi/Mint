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
#include "utility/Assert.hpp"

#include "llvm/Support/Casting.h"

namespace mint {
/*
  #NOTE: normally I would be happy to simply use the llvm::*
  provided functions. I just want the program to break exactly
  when an assert is hit, and not later. which is only applicable
  for 'cast'. The other functions are wrapped only for uniformity.
*/

template <class To, class From>
[[nodiscard]] inline auto isa(From *value) noexcept {
  return llvm::isa<To>(value);
}

template <class To, class From>
[[nodiscard]] inline auto cast(From *value) noexcept {
  To *result = llvm::dyn_cast<To>(value);
  MINT_ASSERT(result != nullptr);
  return result;
}

template <class To, class From>
[[nodiscard]] inline auto dyn_cast(From *value) noexcept {
  return llvm::dyn_cast<To>(value);
}
} // namespace mint
