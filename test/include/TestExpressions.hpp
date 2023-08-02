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
#include <array>
#include <string_view>

#include "utility/Config.hpp"

struct TestCode {
  std::string_view setup;
  std::string_view test_code;
  std::string_view expected_result;

  constexpr TestCode(std::string_view test_code,
                     std::string_view expected_result) noexcept
      : setup(), test_code(test_code), expected_result(expected_result) {}

  constexpr TestCode(std::string_view setup, std::string_view test_code,
                     std::string_view expected_result) noexcept
      : setup(setup), test_code(test_code), expected_result(expected_result) {}
};

/*
#TODO: generate random input for test expressions
#TODO: test edge cases and undefined behavior
*/
constexpr inline auto getAllTestCode() noexcept {
  TestCode expressions[] = {
      {"nil;", "nil"},
      {"1;", "1"},
      {"true;", "true"},
      {"false;", "false"},
      {"!true;", "false"},
      {"-1;", "-1"},
      {"1 + 1;", "2"},
      {"2 - 1;", "1"},
      {"2 * 2;", "4"},
      {"2 / 2;", "1"},
      {"2 % 2;", "0"},
      {"2 == 2;", "true"},
      {"2 != 2;", "false"},
      {"3 > 1;", "true"},
      {"3 >= 1;", "true"},
      {"3 < 1;", "false"},
      {"3 <= 1;", "false"},
      {"true & true;", "true"},
      {"false | true;", "true"},
      {"true == false;", "false"},
      {"false != false;", "false"},
      {"public let a = 1;", "a;", "1"},
      {"public let a = 1;\n public let b = a;", "b;", "1"},
      {"public let b = a;\n public let a = 1;", "b;", "1"},
      {"public let b = a;\n public let a = c;\n public let c = 1;", "b;", "1"},
      {"module A {\n public let a = 1; \n}", "A::a;", "1"},
      {"module A {\n public let a = 1; \n public let b = a; \n}", "A::b;", "1"},
      {"module A {\n public let b = 1; \n public let a = 1; \n}", "A::b;", "1"},
      {"module A {\n public let b = a; \n public let a = c; \n public let c = "
       "1; \n}",
       "A::b;", "1"},
      {"public let a = 1; \n module A {\n public let a = ::a; \n}", "A::a;",
       "1"},
      {"module A {\n public let a = ::a; \n}\n public let a = 1;", "A::a;",
       "1"},
      {"module A {\n public let b = 1; \n}\n module A {\n public let a = b; "
       "\n}",
       "A::a;", "1"},
      {"module A {\n public let a = b; \n}\n module A {\n public let b = 1; "
       "\n}",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n}\n module B {\n public let a = "
       "1; \n}",
       "A::a;", "1"},
      {"module B {\n public let a = 1; \n} module A {\n public let a = B::a; "
       "\n}",
       "A::a;", "1"},
      {"module A {\n module B {\n public let a = 1; }\n public let a = B::a; }",
       "A::a;", "1"},
      {"module A {\n module B {\n public let a = 1; }\n } module A{ public let "
       "a = B::a; }",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n module B {\n public let a = 1; "
       "}\n}",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n} module A { module B { public let "
       "a = 1; }}",
       "A::a;", "1"},
      //{"import \"" MINT_RESOURCES_DIR "/module.mi\";", "::A::a;", "1"},
      //{"import \"" MINT_RESOURCES_DIR "/module.mi\";", "::a;", "2"},
      //{"import \"" MINT_RESOURCES_DIR "/module.mi\";", "::A::b;", "2"},
      //{"import \"" MINT_RESOURCES_DIR "/module.mi\";", "::A::d;", "3"},
      //{"import \"" MINT_RESOURCES_DIR "/module.mi\";", "::A::e;", "4"},
  };

  return std::to_array(expressions);
}