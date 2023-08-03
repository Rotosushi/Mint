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
#include "codegen/Allocate.hpp"
#include "adt/Environment.hpp"

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"

namespace mint {
auto createLLVMGlobalVariable(Environment &env, std::string_view name,
                              llvm::Type *type, llvm::Constant *init) noexcept
    -> llvm::GlobalVariable * {
  auto variable = env.getOrInsertGlobal(name, type);

  if (init != nullptr)
    variable->setInitializer(init);

  return variable;
}
} // namespace mint
