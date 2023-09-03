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
#include <string_view>

#include "llvm/IR/Constant.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace mint {
class Environment;

auto createLLVMLocalVariable(Environment &env, std::string_view name,
                             llvm::Type *type,
                             llvm::Value *init = nullptr) noexcept
    -> llvm::AllocaInst *;

auto createLLVMGlobalVariable(Environment &env, std::string_view name,
                              llvm::Type *type,
                              llvm::Constant *init = nullptr) noexcept
    -> llvm::GlobalVariable *;

auto createLLVMVariable(Environment &env, std::string_view name,
                        llvm::Type *type, llvm::Value *init = nullptr) noexcept
    -> llvm::Value *;
} // namespace mint
