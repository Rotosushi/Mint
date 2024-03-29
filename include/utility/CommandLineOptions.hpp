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
#include <filesystem>

namespace fs = std::filesystem;

#include "utility/Config.hpp"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace cl = llvm::cl;

//  https://llvm.org/docs/CommandLine.html#quick-start-guide
namespace mint {
inline cl::list<std::string> input_files(cl::Positional,
                                         cl::desc("<input file>"));

inline cl::opt<std::string> output_file("o",
                                        cl::desc("specify the output filename"),
                                        cl::value_desc("<output file>"));

inline cl::list<std::string>
    include_paths("I", cl::desc("add an include path to the search space"),
                  cl::value_desc("path"));

enum class EmittedFiletype {
  NativeOBJ,
  NativeASM,
  LLVM_IR,
  // MintIR?
  // NativeStaticLibrary,
  // NativeDynamicLibrary,
};

inline cl::opt<EmittedFiletype> emittedFiletype(
    "emit", cl::desc("select the kind of file to emit"),
    cl::values(clEnumValN(EmittedFiletype::LLVM_IR, "llvm-ir",
                          "emit in llvm's intermediate representation"),
               clEnumValN(EmittedFiletype::NativeASM, "asm",
                          "emit in native asm"),
               clEnumValN(EmittedFiletype::NativeOBJ, "obj",
                          "emit a native object file")));

inline void printVersion(llvm::raw_ostream &out) noexcept {
  out << "mint version: " << MINT_VERSION_MAJOR << "." << MINT_VERSION_MINOR
      << "." << MINT_VERSION_PATCH << "\n git revision [" << MINT_GIT_REVISION
      << "]\n Compiled on " << __DATE__ << " at " << __TIME__ << "\n";
}
} // namespace mint
