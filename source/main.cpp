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

#include "comptime/Compile.hpp"
#include "comptime/Link.hpp"
#include "comptime/Repl.hpp"
#include "utility/CommandLineOptions.hpp"

#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"

auto main(int argc, char **argv) -> int {
  llvm::InitLLVM llvm{argc, argv};
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetDisassembler();

  llvm::cl::SetVersionPrinter(mint::printVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  if (mint::input_files.empty()) {
    return mint::repl();
  }
  std::vector<fs::path> input_paths;
  for (auto &path : mint::input_files) {
    input_paths.emplace_back(std::move(path));
  }

  if (mint::compile(input_paths) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (mint::output_file.empty()) {
    mint::output_file = (fs::current_path() /= "a.out").string();
  }

  if (mint::emittedFiletype == mint::EmittedFiletype::NativeOBJ) {
    std::vector<fs::path> object_filenames;
    for (const auto &filename : input_paths) {
      fs::path object_filename = filename;
      object_filename.replace_extension(".o");
      object_filenames.emplace_back(std::move(object_filename));
    }

    if (mint::link(std::cerr, object_filenames, mint::output_file) ==
        EXIT_FAILURE) {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}