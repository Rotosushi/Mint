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
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace fs = std::filesystem;

#include "adt/Environment.hpp"
#include "utility/Config.hpp"
#include "utility/Process.hpp"

struct CompiledCodeTestFile {
  std::string_view filename;
  int expected_result;

  constexpr CompiledCodeTestFile(std::string_view filename,
                                 int expected_result) noexcept
      : filename(filename), expected_result(expected_result) {}
};

constexpr inline auto getCompiledCodeTestFiles() noexcept {
  CompiledCodeTestFile expressions[] = {
      {"exit_code.mi", 42},      {"arithmetic.mi", 3},
      {"local_variables.mi", 4}, {"global_variables.mi", 24},
      {"modules.mi", 6},
  };
  return std::to_array(expressions);
}

inline bool testFile(CompiledCodeTestFile &x) {
  static const char *mint_path = MINT_BUILD_DIR "/source/mint";

  bool success = true;
  fs::path filepath{MINT_RESOURCES_DIR "/test_files"};
  filepath /= x.filename;
  MINT_ASSERT(fs::exists(filepath));

  fs::path compiled_code_path{filepath};
  compiled_code_path.replace_extension();

  std::vector<const char *> mint_arguments{
      mint_path,
      "-o",
      compiled_code_path.c_str(),
      filepath.c_str(),
  };

  if (mint::process(mint_path, mint_arguments) != 0) {
    std::cerr << "Compilation of file [" << filepath << "] failed.\n";
    success = false;
    return success;
  }

  std::vector<const char *> compiled_code_arguments{
      compiled_code_path.c_str(),
  };

  int result =
      mint::process(compiled_code_path.c_str(), compiled_code_arguments);

  if (result != x.expected_result) {
    std::cerr << "file [" << filepath << "] failed.\nExpected ["
              << x.expected_result << "], Actual [" << result << "]\n";
    success = false;
  }

  std::error_code errc;
  if (fs::remove(compiled_code_path, errc) == false) {
    std::cerr << "Unable to delete built file [" << compiled_code_path << "]"
              << " error [" << errc << "]\n";
  }

  return success;
}

TEST_CASE("Test Compiled Code", "[integration]") {
  auto test_files = getCompiledCodeTestFiles();
  for (auto &file : test_files) {
    CHECK(testFile(file));
  }
}