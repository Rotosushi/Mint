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
  https://www.boost.org/doc/libs/1_82_0/libs/test/doc/html/boost_test/adv_scenarios/single_header_customizations/entry_point.html
*/
#define BOOST_TEST_MODULE mint
#include "boost/test/unit_test.hpp"

/*
#NOTE:
there is an unfavorable interaction with boost when compiling with
-fno-exceptions for whatever reason, and this function needs to be
defined in that case, along with the macro.
however I am fine having exceptions enabled within mint.
they were turned off only because of using
llvm-config --cxxflags to generate compilation flags for llvm compatibility.
however, these don't seem to be strictly necessary when linking against
llvm statically.

#define BOOST_NO_EXCEPTIONS
#include "boost/throw_exception.hpp"
void boost::throw_exception([[maybe_unused]] std::exception const &e) {
  std::terminate();
}
*/

BOOST_AUTO_TEST_CASE(hello_boost_test) { BOOST_TEST(true); }
