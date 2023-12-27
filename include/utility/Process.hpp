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
#include <utility>
#include <vector>

#include "utility/Abort.hpp"
#include "utility/Config.hpp"

#if defined(MINT_HOST_OS_LINUX)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
namespace mint {
inline int process(const char *pathname, std::vector<const char *> &arguments) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("a call to fork() failed");
    abort("a call to fork() failed");
  } else if (pid == 0) {
    // child process
    execvp(pathname, const_cast<char *const *>(arguments.data()));
    // #NOTE: unreachable on successful call to execvp
    perror("a call to execvp(...) failed");
    abort("a call to execvp(...) failed");
  } else {
    // parent process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
      perror("a call to waitpid(...) failed.");
      abort("a call to waitpid(...) failed.");
    }

    // #TODO: I think I prefer the following,
    // except that EXIT_FAILURE expands to 1
    // not -1. so we cannot distingush between
    // a error program, and
    // if (WEXITED(status))
    //   return WEXITSTATUS(status);
    // else
    //   return EXIT_FAILURE;
    return WEXITSTATUS(status);
  }
}
} // namespace mint
#else
#error "unsupported OS"
#endif
