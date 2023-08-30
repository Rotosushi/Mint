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
#define BOOST_TEST_DYN_LINK
#include "boost/test/unit_test.hpp"

#include <sstream>

#include "TestExpressions.hpp"
#include "adt/Environment.hpp"
#include "core/Repl.hpp"

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

void testExpressionInREPL(TestCode &expression) {
  std::stringstream input;
  if (!expression.setup.empty())
    input << expression.setup << "\n";
  input << expression.test_code << "\n";
  // input.put('\0');
  [[maybe_unused]] auto inview = input.view();
  std::stringstream output;
  std::stringstream error_output;
  std::stringstream log_output;
  auto env =
      mint::Environment::create(&input, &output, &error_output, &log_output);

  auto failed = repl(env, true);
  BOOST_REQUIRE(failed == EXIT_SUCCESS);

  auto outview = output.view();
  auto line = extractFinalLine(outview);
  auto result = extractResult(line);

  auto success = result == expression.expected_result;
  BOOST_CHECK(success);
  if (!success) {
    std::cerr << "TestCase {\n";

    if (expression.setup.size() > 0)
      std::cerr << "Setup: " << expression.setup << "\n";

    std::cerr << "Test Expression: " << expression.test_code
              << "\n Expected Result: " << expression.expected_result
              << "\n Actual Result: " << result
              << "\n}\nError: " << error_output.view() << "\n";
  }
}

BOOST_AUTO_TEST_CASE(mint_repl) {
  auto test_expressions = getAllTestCode();

  for (auto &expression : test_expressions) {
    testExpressionInREPL(expression);
  }
}