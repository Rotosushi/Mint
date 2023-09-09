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
#include "adt/Environment.hpp"
#include "comptime/Codegen.hpp"
#include "comptime/Emit.hpp"
#include "comptime/Evaluate.hpp"
#include "comptime/Parse.hpp"
#include "comptime/Typecheck.hpp"
#include "utility/CommandLineOptions.hpp"

namespace mint {
[[nodiscard]] int compile(fs::path file);

[[nodiscard]] int compile(std::vector<std::string> const &filenames) {
  // #TODO: spawn a thread for each file
  // #TODO: link all intermediate files together based on
  // given cli flags.
  for (auto &filename : filenames) {
    if (compile(filename) == EXIT_FAILURE) {
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

[[nodiscard]] int compile(fs::path file) {
  auto env = Environment::create();
  // #TODO: move this into Environment::create()
  env.appendDirectories(include_paths);

  if (parse(file, env) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (typecheck(env) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (evaluate(env) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (codegen(env) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (emit(file, env) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
} // namespace mint