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

#if !defined(NDEBUG)
#  include "utility/Log.hpp" // mint::log
#  include <iostream>        // std::cerr
#  define MINT_ASSERT(condition)                                               \
    (condition) ? (1)                                                          \
                : (mint::log(std::cerr, #condition), (MINT_BREAKPOINT(), 0))
#else
#  define MINT_ASSERT(condition)
#endif
