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
#include "codegen/Store.hpp"
#include "adt/Environment.hpp"

namespace mint {
// https://llvm.org/docs/LangRef.html#store-instruction
auto createLLVMStore(Environment &env, llvm::Value *source,
                     llvm::Value *target) noexcept -> llvm::Value * {
  // #NOTE: we cannot store types which are larger than
  // a single word on the target machine.
  // #NOTE: none of the currently available types in the 
  // language have a representation larger than a single 
  // word.
  return env.createLLVMStore(source, target);
}
} // namespace mint
