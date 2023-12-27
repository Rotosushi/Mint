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
#include "runtime/sys/Exit.hpp"
#include "runtime/InlineAsm.hpp"

#include "adt/Environment.hpp"

namespace mint {
void sysExit(Environment &env, llvm::Value *exit_code) noexcept {
  auto *size_type = env.getLLVMSizeType();
  auto *void_type = env.getLLVMVoidType();
  auto *mov_rax_type = env.getLLVMFunctionType(size_type, {});
  auto *mov_rdi_type = env.getLLVMFunctionType(size_type, {size_type});
  auto *syscall_type = env.getLLVMFunctionType(void_type, {});

  auto *result_value = env.createLLVMSExt(exit_code, size_type);

  auto *mov_rdi = [&]() {
    if (llvm::isa<llvm::ConstantInt>(exit_code)) {
      return createInlineAsm(env, mov_rdi_type, "mov rdi, $1", "={rdi},i");
    } else {
      return createInlineAsm(env, mov_rdi_type, "mov rdi, $1", "={rdi},r");
    }
  }();
  auto *mov_rax = createInlineAsm(env, mov_rax_type, "mov rax, 60", "={rax}");
  auto *syscall = createInlineAsm(env, syscall_type, "syscall", "");

  env.createLLVMCall(mov_rdi, {result_value});
  env.createLLVMCall(mov_rax, {});
  env.createLLVMCall(syscall, {});
}
} // namespace mint
