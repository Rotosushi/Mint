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
#include <cstdlib>

#include "utility/Assert.hpp"
#include "utility/OptionsParser.hpp"

#include "type/Type.hpp"

#include "ast/Ast.hpp"

auto main(int argc, char **argv) -> int {
  mint::OptionsParser options_parser{argc, argv};
  options_parser.parse();

  mint::TypeInterner interner;
  auto t0 = interner.getBooleanType();
  auto t1 = interner.getIntegerType();
  auto t2 = interner.getIntegerType();
  auto t3 = interner.getNilType();

  MINT_ASSERT(t0 != t1);
  MINT_ASSERT(t1 == t2);
  MINT_ASSERT(t0 != t3);
  MINT_ASSERT(t1 != t3);

  MINT_ASSERT(!mint::equals(t0, t1));
  MINT_ASSERT(mint::equals(t1, t2));
  MINT_ASSERT(!mint::equals(t0, t3));
  MINT_ASSERT(!mint::equals(t1, t3));

  std::cout << t0 << "\n" << t1 << "\n" << t2 << "\n" << t3 << "\n";

  return EXIT_SUCCESS;
}