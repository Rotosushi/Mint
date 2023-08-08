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
#include <cstdlib>
#include <iostream>

#include "adt/Environment.hpp"
#include "core/Core.hpp"
#include "utility/CommandLineOptions.hpp"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"

auto main(int argc, char **argv) -> int {
  llvm::InitLLVM llvm{argc, argv};
  llvm::cl::SetVersionPrinter(mint::printVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetDisassembler();

  mint::Environment env = mint::Environment::create();

  if (mint::input_filename.empty())
    return repl(env, true);
  else {
    env.sourceFile(mint::input_filename.c_str());
    return compile(env);
  }
}