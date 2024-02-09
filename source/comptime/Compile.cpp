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
#include <iostream>
#include <utility>

#include "adt/Environment.hpp"
#include "adt/ThreadPool.hpp"
#include "comptime/Codegen.hpp"
#include "comptime/Compile.hpp"
#include "comptime/Emit.hpp"
#include "comptime/Evaluate.hpp"
#include "comptime/Parse.hpp"
#include "comptime/Typecheck.hpp"

#include "utility/CommandLineOptions.hpp"
#include "utility/VerifyLLVM.hpp"

namespace mint {
void compile(UniqueFile &file, UniqueFiles &files);

[[nodiscard]] int compile(UniqueFiles &filenames) {
  // #TODO: spawn a thread for each file
  // #TODO: link all intermediate files together based on
  // given cli flags.

  // #NOTE: the expected behavior is that only
  // when every file in the set is done, do we
  // stop attempting to compile files. if a file
  // is unprocessed, then we start compiling it.
  // (this is the point where a job is added to the
  // thread pool) if a file is in_progress, then it
  // has the potential to add a new file to the set
  // of UniqueFiles.

  bool done = false;
  bool failed = false;
  ThreadPool pool;
  pool.start();

  while (!done) {
    done = true;
    for (auto &file : filenames) {
      if (file.unprocessed()) {
        done = false;
        pool.queue([&] { compile(file, filenames); });
      } else if (file.in_progress()) {
        done = false;
      } else if (file.failed()) {
        failed = true;
      }
      // #NOTE: we do not check for file.done() explicitly,
      // as we only want that flag to be true when all files
      // are done (or failed). Which is handled by setting
      // done to true in the outer loop.
    }
  }

  pool.stop();

  if (!failed) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

void compile(UniqueFile &file, UniqueFiles &files) {
  auto env = Environment::create(files);

  file.state(UniqueFile::InProgress);

  if (parse(file.path(), env) == EXIT_FAILURE) {
    file.state(UniqueFile::Failed);
    return;
  }

  if (typecheck(env) == EXIT_FAILURE) {
    file.state(UniqueFile::Failed);
    return;
  }

  if (evaluate(env) == EXIT_FAILURE) {
    file.state(UniqueFile::Failed);
    return;
  }

  if (codegen(env) == EXIT_FAILURE) {
    file.state(UniqueFile::Failed);
    return;
  }

  // #NOTE: verify returns true on failure,
  // intended for use within an early return if statement.
  // If the emitted assembly fails verification, that is an
  // error with the compiler and not the source code.
  MINT_ASSERT(!verify(env.getLLVMModule(), env.errorStream()));

  if (emittedFiletype == EmittedFiletype::NativeOBJ) {
    if (emitELF(file.path(), env) == EXIT_FAILURE) {
      file.state(UniqueFile::Failed);
      return;
    }
  } else if (emittedFiletype == EmittedFiletype::NativeASM) {
    if (emitX86Assembly(file.path(), env) == EXIT_FAILURE) {
      file.state(UniqueFile::Failed);
      return;
    }
  } else {
    if (emitLLVMIR(file.path(), env) == EXIT_FAILURE) {
      file.state(UniqueFile::Failed);
      return;
    }
  }

  file.state(UniqueFile::Done);
  return;
}
} // namespace mint
