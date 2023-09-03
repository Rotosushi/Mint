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
#include "core/Compile.hpp"
#include "adt/Environment.hpp"
#include "core/Codegen.hpp"
#include "core/Repl.hpp"

namespace mint {
[[nodiscard]] int compile(Environment &env) {
  auto &err = env.getErrorStream();
  auto &source_file = env.sourceFile();
  if (!source_file) {
    err << "No source to compile.\n";
    return EXIT_SUCCESS;
  }

  env.pushActiveSourceFile(source_file.value());
  if (repl(env, false) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  for (auto &mir : env.getModule()) {
    auto result = codegen(mir, env);
    if (!result) {
      if (result.recovered()) {
        continue;
      }

      auto error = result.error();
      err << error;
      return EXIT_FAILURE;
    }
  }

  return env.emitLLVMIR();
}
} // namespace mint
