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
#include <sstream>

#include "runtime/InlineAsm.hpp"

#include "adt/Environment.hpp"
#include "utility/PrintLLVM.hpp"

namespace mint {
llvm::InlineAsm *createInlineAsm(Environment &env, llvm::FunctionType *asm_type,
                                 std::string_view asm_string,
                                 std::string_view constraint_string,
                                 bool has_side_effects, bool is_align_stack,
                                 llvm::InlineAsm::AsmDialect asm_dialect,
                                 bool can_throw) {
  if (auto error = llvm::InlineAsm::verify(asm_type, constraint_string)) {
    std::stringstream buffer;
    buffer << "InlineAsm constraint string invalid [" << constraint_string
           << "], for llvm::Type ["
           << "]. llvm::InlineAsm::verify error [" << error << "]\n";
    abort(buffer.view());
  }

  return llvm::InlineAsm::get(asm_type, asm_string, constraint_string,
                              has_side_effects, is_align_stack, asm_dialect,
                              can_throw);
}
} // namespace mint
