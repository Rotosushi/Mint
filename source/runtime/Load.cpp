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
#include "runtime/Load.hpp"
#include "adt/Environment.hpp"

namespace mint {
auto createLLVMLoad(Environment &env, llvm::Type *type,
                    llvm::Value *source) noexcept -> llvm::Value * {
  // #NOTE: llvm immediate value's cannot be loaded.
  // as load expects a pointer type. And as far as I can tell
  // llvm::Argument, and llvm::ConstantData are the only
  // immediate values produced by the compiler as of now.
  // and we need to emit a load to access the value of a
  // global variable.
  if ((llvm::isa<llvm::Argument>(source)) ||
      (llvm::isa<llvm::Constant>(source) &&
       !llvm::isa<llvm::GlobalVariable>(source))) {
    return source;
  }

  // #NOTE: we cannot create a load instruction for a type which
  // does not fit within a single register.
  // https://llvm.org/docs/LangRef.html#load-instruction
  // https://llvm.org/docs/LangRef.html#single-value-types
  // #NOTE: none of the available types (Integer, Boolean, Nil, Lambda)
  // are larger than a single register.
  // #NOTE: Lambda is the size of a function pointer
  return env.createLLVMLoad(type, source);
}
} // namespace mint
