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
        while (!isspace(*cursor)) {
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

bool testExpressionInREPL(TestExpression &expression) {
  std::stringstream input(std::string(expression.test_code));
  std::stringstream output;
  auto env = mint::Environment::create(&input, &output);

  env.repl();

  auto result = extractResult(output.view());

  return result == expression.expected_result;
}

BOOST_AUTO_TEST_CASE(mint_repl) {
  auto test_expressions = getTestExpressions();

  for (auto &expression : test_expressions) {
    BOOST_CHECK(testExpressionInREPL(expression));
  }
}