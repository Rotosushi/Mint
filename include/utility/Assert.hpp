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

#if defined(__clang__) || defined(__GNUC__)
#  define MINT_BREAKPOINT() __builtin_trap()
#elif defined(__MSC_VER__)
#  include <intrin.h>
#  define MINT_BREAKPOINT() __debugbreak()
#else
#  include <cstdlib>
#  define MINT_BREAKPOINT() std::abort()
#endif

/*
  #NOTE: the use of operator , in the false case of the MINT_ASSERT
  macro. operator , simply evaluates it's left hand side throws away
  the result, then evaluates it's right hand side and returns that
  as the result of the expression. Thus the log occurs, and then the
  MINT_BREAKPOINT instruction is executed, halting the process.
  then in order for the type of the ternary operator ? : to be correct
  we return 1, to match the result of the true condition.
*/
#if !defined(NDEBUG)
#  include "utility/Log.hpp" // mint::log
#  include <iostream>        // std::cerr
#  define MINT_ASSERT(condition)                                               \
    (condition) ? (1)                                                          \
                : (mint::log(std::cerr, #condition), (MINT_BREAKPOINT(), 0))
#else
#  define MINT_ASSERT(condition)
#endif
