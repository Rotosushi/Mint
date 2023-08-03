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
#include "adt/Environment.hpp"
#include "codegen/Allocate.hpp"

namespace mint {
auto createLLVMLoad(Environment &env, llvm::Type *type,
                    llvm::Value *source) noexcept -> llvm::Value * {
  // #NOTE: we cannot create a load instruction for a type which
  // does not fit within a single register.
  // https://llvm.org/docs/LangRef.html#load-instruction
  // https://llvm.org/docs/LangRef.html#single-value-types
  // #NOTE: none of the available types (Integer, Boolean, Nil)
  // are larger than a single register.
  return env.createLLVMLoad(type, source);
}
} // namespace mint
