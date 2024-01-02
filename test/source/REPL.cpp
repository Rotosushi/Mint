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

/*
  okay, so how do we test the REPL?

  the basic interface to the repl is the
  istream and ostream that it reads from and
  writes to.

  so we can hook the input stream up to
  a stringstream which has been filled with
  the test code.

  and we can hook the output stream up to
  a stringstream which we parse to retrieve
  the output.

  okay, and how do we write a function which
  'knows' when we want to parse a integer from
  the result vs a boolean vs a nil?

  templates?
  a different function body per test?

  and how do we write a function which needs
  to test multiple lines of input into the
  REPL before it tests the output?

  loops?
  a different function body per test?

  okay, how do we write a set of test parameters
  such that we can input these same parameters into
  a test of the REPL or a test of the upcoming compiler?

  some sort of struct which holds the test code along with
  the expected result.

  well how do we not tie this parameter struct to either
  the repl or the compilation process?
*/
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <sstream>
#include <string_view>

#include "adt/Environment.hpp"
#include "comptime/Repl.hpp"
#include "utility/Config.hpp"

struct ReplTestCode {
  std::string_view setup;
  std::string_view test_code;
  std::string_view expected_result;

  constexpr ReplTestCode(std::string_view test_code,
                         std::string_view expected_result) noexcept
      : setup(), test_code(test_code), expected_result(expected_result) {}

  constexpr ReplTestCode(std::string_view setup, std::string_view test_code,
                         std::string_view expected_result) noexcept
      : setup(setup), test_code(test_code), expected_result(expected_result) {}
};

/*
#TODO: generate random input for test expressions
#TODO: test edge cases and undefined behavior
*/
constexpr inline auto getReplTestCode() noexcept {
  ReplTestCode expressions[] = {
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
      {"module A {\n public let b = a; \n public let a = c; "
       "\n public let c = 1; \n}",
       "A::b;", "1"},
      {"public let a = 1; \n module A {\n public let a = ::a; \n}", "A::a;",
       "1"},
      {"module A {\n public let a = ::a; \n}\n public let a = 1;", "A::a;",
       "1"},
      {"module A {\n public let b = 1; \n}\n "
       "module A {\n public let a = b; \n}",
       "A::a;", "1"},
      {"module A {\n public let a = b; \n}\n "
       "module A {\n public let b = 1; \n}",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n}\n"
       " module B {\n public let a = 1; \n}",
       "A::a;", "1"},
      {"module B {\n public let a = 1; \n} "
       " module A {\n public let a = B::a;\n}",
       "A::a;", "1"},
      {"module A {\n module B {\n public let a = 1; }\n "
       " public let a = B::a; } ",
       "A::a;", "1"},
      {"module A {\n module B {\n public let a = 1; }\n }"
       " module A{ public let a = B::a; }",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n"
       "module B {\n public let a = 1; }\n}",
       "A::a;", "1"},
      {"module A {\n public let a = B::a; \n}\n"
       "module A { module B { public let a = 1; }}",
       "A::a;", "1"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::b;", "1"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "a;", "1"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::c;", "2"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "d;", "2"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::e;", "3"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::a;", "3"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "g;", "3"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::f;", "4"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::g;", "4"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::h;", "5"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::B::i;", "5"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::B::k;", "6"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::j;", "6"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::m;", "7"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::B::l;", "7"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::o;", "8"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::B::n;", "8"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "q;", "9"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::p;", "9"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::s;", "10"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::r;", "10"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::u;", "11"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "A::t;", "11"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::C::w;", "12"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::v;", "12"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::z;", "13"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::C::x;", "13"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::C::a;", "14"},
      {"import \"" MINT_RESOURCES_DIR "/module.mi\";", "B::C::y;", "14"},
      {"fn f() {}", "f();", "nil"},
      {"fn f(a : Nil) { a; }", "f(nil);", "nil"},
      {"fn f(a: Boolean) { a; }", "f(true);", "true"},
      {"fn f(a: Integer) { a; }", "f(2);", "2"},
      {"fn f(a: Boolean) { !a; }", "f(true);", "false"},
      {"fn f(a: Integer) { -a; }", "f(3);", "-3"},
      {"fn f(a: Integer, b: Integer) { a + b; }", "f(2, 5);", "7"},
      {"fn f(a: Integer, b: Integer) { a - b; }", "f(7, 5);", "2"},
      {"fn f(a: Integer, b: Integer) { a * b; }", "f(7, 5);", "35"},
      {"fn f(a: Integer, b: Integer) { a / b; }", "f(35, 7);", "5"},
      {"fn f(a: Integer, b: Integer) { a % b; }", "f(7, 5);", "2"},
      {"fn f(a: Integer, b: Integer) { a == b; }", "f(3, 3);", "true"},
      {"fn f(a: Integer, b: Integer) { a != b; }", "f(3, 3);", "false"},
      {"fn f(a: Boolean, b: Boolean) { a == b; }", "f(true, true);", "true"},
      {"fn f(a: Boolean, b: Boolean) { a != b; }", "f(true, true);", "false"},
      {"fn f(a: Integer, b: Integer) { a > b; }", "f(4, 5);", "false"},
      {"fn f(a: Integer, b: Integer) { a >= b; }", "f(5, 5);", "true"},
      {"fn f(a: Integer, b: Integer) { a < b; }", "f(3, 4);", "true"},
      {"fn f(a: Integer, b: Integer) { a <= b; }", "f(4, 4);", "true"},
      {"fn f(a: Boolean, b: Boolean) { a & b; }", "f(true, true);", "true"},
      {"fn f(a: Boolean, b: Boolean) { a | b; }", "f(true, false);", "true"},
      {"fn f(a: Integer, b: Integer) { a + b; } \n"
       " fn g(a: Integer) { f(a, a); }",
       "g(2);", "4"},
      {"fn g(a: Integer) { f(a, a); }\n"
       "fn f(a: Integer, b: Integer) { a + b; }",
       "g(3);", "6"},
      {"module A {\n public fn f(a: Integer) { a + a; } \n}", "A::f(2);", "4"},
  };

  return std::to_array(expressions);
}

/*
  #NOTE: this function is rather inelegant
  it's goal is simply to extract the result value
  from the output of the repl.
  after each expression is parsed, typechecked and evaluated
  the repl emits
  <ast> : <type> => <value> \n
  #

  this means we simply need to extract the string from
  "=>" to "\n".
*/
std::string_view extractResult(std::string_view view) {
  auto cursor = view.begin();
  auto token = view.begin();
  auto end = view.end();
  while (cursor != end) {
    // search for "=>"
    if (*cursor == '=') {
      token = cursor;
      ++token;
      if (*token == '>') {
        // found "=>"
        ++token;
        cursor = token;
        // ignore whitespace
        while (isspace(*cursor)) {
          ++cursor;
          ++token;
        }
        // walk until we hit the newline
        while (*cursor != '\n') {
          ++cursor;
        }

        return {token, static_cast<std::size_t>(std::distance(token, cursor))};
      }
    }

    ++cursor;
  }

  // we somehow reached the end of the buffer, return
  // the empty string. this should cause the test to fail.
  return {cursor, end};
}

/*
  #NOTE: walk backwards through the view until we see a '\n'
  store this location (1), then keep walking backwards until we
  see another '\n' or the end of the view, store this location (2)
  return a view from 2 to 1.
*/
std::string_view extractFinalLine(std::string_view view) noexcept {
  auto cursor = view.rbegin();
  auto token = view.rbegin();
  auto end = view.rend();

  while (cursor != end) {
    if (*cursor == '\n') {
      token = cursor;
      ++token;

      while (token != end) {
        if (*token == '\n') {
          break;
        }
        ++token;
      }

      return {token.base(), static_cast<std::size_t>(
                                std::distance(token.base(), cursor.base()))};
    }

    ++cursor;
  }

  // there was never a '\n' in the view, return the whole view
  return {cursor.base(),
          static_cast<std::size_t>(std::distance(cursor.base(), end.base()))};
}

TEST_CASE("Test Repl", "[integration]") {
  auto test_expressions = getReplTestCode();

  for (auto &expression : test_expressions) {
    std::stringstream input;
    if (!expression.setup.empty())
      input << expression.setup << "\n";
    input << expression.test_code << "\n";
    // input.put('\0');
    [[maybe_unused]] auto inview = input.view();
    std::stringstream output;
    std::stringstream error_output;
    std::stringstream log_output;

    auto failed = mint::repl(&input, &output, &error_output, &log_output);
    REQUIRE(failed == EXIT_SUCCESS);

    auto outview = output.view();
    auto line = extractFinalLine(outview);
    auto result = extractResult(line);

    auto success = result == expression.expected_result;
    CHECK(success);
    if (!success) {
      std::cerr << "TestCase {\n";

      if (expression.setup.size() > 0) {
        std::cerr << "Setup: " << expression.setup << "\n";
      }

      std::cerr << "Test Expression: " << expression.test_code
                << "\n Expected Result: " << expression.expected_result
                << "\n Actual Result: " << result
                << "\n}\nError: " << error_output.view() << "\n";
    }
  }
}