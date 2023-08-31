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
#include "codegen/LLVMUtility.hpp"

#include "llvm/Support/raw_ostream.h"

namespace mint {
auto emitLLVMIR(llvm::Module &module, fs::path const &filename,
                std::ostream &error_output) noexcept -> int {
  auto llvm_ir_filename = filename;
  llvm_ir_filename.replace_extension("ll");
  std::error_code error;
  llvm::raw_fd_ostream outfile{llvm_ir_filename.c_str(), error};
  if (error) {
    error_output << "Couldn't open file: " << llvm_ir_filename << " -- "
                 << error << "\n";
    return EXIT_FAILURE;
  }

  module.print(outfile, /* AssemblyAnnotationWriter = */ nullptr);
  return EXIT_SUCCESS;
}

auto toString(llvm::Error const &error) noexcept -> std::string {
  std::string result;
  llvm::raw_string_ostream stream{result};
  stream << error;
  return result;
}

auto toString(llvm::Type const *type) noexcept -> std::string {
  std::string result;
  llvm::raw_string_ostream stream{result};
  type->print(stream);
  return result;
}

auto toString(llvm::Value const *value) noexcept -> std::string {
  std::string result;
  llvm::raw_string_ostream stream{result};
  value->print(stream);
  return result;
}
} // namespace mint
