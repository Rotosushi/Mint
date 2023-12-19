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
#include "comptime/Emit.hpp"

#include "adt/Environment.hpp"

#include "llvm/IR/LegacyPassManager.h"

namespace mint {
static int file_open_failed(std::ostream &errout, fs::path const &path,
                            std::error_code const &errc) noexcept {
  errout << "Could not open file: " << path << " -- " << errc << "\n";
  return EXIT_FAILURE;
}

static int adding_passes_failed(std::ostream &errout,
                                std::string_view file_kind,
                                llvm::TargetMachine &tm) noexcept {
  errout << "Cannot write " << file_kind << " file for Target Machine \n["
         << tm.getTargetCPU().data() << " " << tm.getTargetTriple().str()
         << "]\n";
  return EXIT_FAILURE;
}

int emitLLVMIR(fs::path const &path, Environment &env) noexcept {
  auto filename = path;
  filename.replace_extension("ll");
  std::error_code errc;
  llvm::raw_fd_ostream outfile(filename.c_str(), errc);
  if (errc) {
    return file_open_failed(env.errorStream(), filename, errc);
  }

  env.printModule(outfile);
  return EXIT_SUCCESS;
}

int emitX86Assembly(fs::path const &path, Environment &env) noexcept {
  auto filename = path;
  filename.replace_extension("s");
  std::error_code errc;
  llvm::raw_fd_ostream outfile{filename.c_str(), errc};
  if (errc) {
    return file_open_failed(env.errorStream(), filename, errc);
  }

  llvm::legacy::PassManager assembly_print_pass;
  bool failed = env.targetMachine().addPassesToEmitFile(
      assembly_print_pass, outfile, nullptr,
      llvm::CodeGenFileType::CGFT_AssemblyFile);

  if (failed) {
    return adding_passes_failed(env.errorStream(), "x86 assembly",
                                env.targetMachine());
  }

  assembly_print_pass.run(env.getLLVMModule());

  return EXIT_SUCCESS;
}

int emitELF(fs::path const &path, Environment &env) noexcept {
  auto filename = path;
  filename.replace_extension(".o");
  std::error_code errc;
  llvm::raw_fd_ostream outfile{filename.c_str(), errc};
  if (errc) {
    return file_open_failed(env.errorStream(), filename, errc);
  }

  llvm::legacy::PassManager elf_print_pass;
  bool failed = env.targetMachine().addPassesToEmitFile(
      elf_print_pass, outfile, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile);

  if (failed) {
    return adding_passes_failed(env.errorStream(), "ELF", env.targetMachine());
  }

  elf_print_pass.run(env.getLLVMModule());

  return EXIT_SUCCESS;
}
} // namespace mint
