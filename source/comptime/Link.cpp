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
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

#include "comptime/Link.hpp"
#include "utility/Abort.hpp"
#include "utility/Process.hpp"

namespace mint {
/*
#NOTE: we only call link when the compiler is expecting to
create an executable or library. so we can assume some things.
-) each given file has been turned into an object file.
-) each object file is right next to the source file
  (we are doing in-source-tree builds for the time being.)

what we cannot assume is that we have the full list of dependent
files just given the list of source files as supplied by the
caller of mint itself. (importing the standard library must happen
implicitly, for instance. This can be extended to the point
where the user only needs to ask to compile the main file, and
this will by definition, compile and link all source files
which main depends upon.)
we are fine assuming that a call to link
supplies all of the needed object files, we just need to have a
step which generates a full listing at some point.
it seems obvious to say, well, just add a function call at the
point where a file is imported which logs the imported source
file, then the list gets built as the program is compiled.
This however comes with some complications, firstly if we are
compiling multiple source files on multiple threads, how do we
synchronize this list of source files between threads, and how
is this data returned to the thread which calls link?
since we are following the model of one Environment per thread,
and the list of source files which has been imported is presumably
stored per environment.
we need to worry about symbols being defined correctly when
they are used in multiple translation units.
i think this is solved by the fact that we parse the imported
file itself when we import into a new TU

not trying to define a symbol twice accross all translation units.
I think this is solved by the fact that we only forward declare any
symbol which is imported.

and supplying each produced object file to the link step.
This is the tricky bit, I think we need a set of filenames
which are visible and modifyable from each thread which has
a translation unit.

as a first step, given a single file which is enough to produce
an exectuable, then we are fine with a more basic form of the
link step.

so, as a starting point, this function will:
-) call the linker with the correct arguments.
  -) specify output filename
  #TODO: -) specify code entry point
  #TODO: -) specify executable, static library, or shared library.
*/
[[nodiscard]] int link(std::ostream &errout,
                       std::vector<fs::path> const &object_filenames,
                       std::string const &output_filename) {
  // auto remove_object_file = [&object_filenames]() {
  //   for (auto const &object_filename : object_filenames) {
  //     std::error_code errc;
  //     if (!fs::remove(object_filename, errc)) {
  //       abort(errc);
  //     }
  //   }
  // };

  if (object_filenames.empty()) {
    errout << "No object files given to link. No work to be done.\n";
    return EXIT_SUCCESS;
  }

  for (auto const &object_filename : object_filenames) {
    if (!fs::exists(object_filename)) {
      errout << "No object file [" << object_filename << "] was found.\n";
      return EXIT_FAILURE;
    }
  }

  // #NOTE: we need to create the output file so that
  // ld can open it.
  std::fstream output_file{output_filename,
                           std::ios_base::out | std::ios_base::trunc};
  output_file.close();

  // #TODO: support more than ld, and don't have the selction hardcoded.
  std::vector<const char *> ld_args = {
      "ld", "-m", "elf_x86_64", "-e", "main", "-o", output_filename.c_str()};
  for (auto const &object_filename : object_filenames) {
    ld_args.push_back(object_filename.c_str());
  }
  ld_args.push_back((const char *)nullptr);

  int result = process("ld", ld_args);
  return result;
}
} // namespace mint
