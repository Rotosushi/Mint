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
#include <sstream>
#include <string_view>

#include "adt/Environment.hpp"
#include "utility/Config.hpp"
#include "utility/Process.hpp"

struct CompiledCodeTestFile {
  std::string_view filename;
  uint8_t expected_result;

  constexpr CompiledCodeTestFile(std::string_view filename,
                                 uint8_t expected_result) noexcept
      : filename(filename), expected_result(expected_result) {}
};

constexpr inline auto getCompiledCodeTestCode() noexcept {
  CompiledCodeTestFile expressions[] = {
      {"fn main() { 42; }", 42U},
  };
  return std::to_array(expressions);
}
