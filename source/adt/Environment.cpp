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
#include "adt/Environment.hpp"

#include "ast/Evaluate.hpp"
#include "ast/Print.hpp"
#include "ast/Typecheck.hpp"

namespace mint {
auto Environment::repl() noexcept -> int {
  bool run = true;

  while (run) {
    *out << "# ";

    auto parse_result = parser.parse();
    if (!parse_result) {
      auto &error = parse_result.error();
      if (error.getKind() == Error::EndOfInput)
        break;
        
      printErrorWithSource(error);
      continue;
    }
    auto &ast = parse_result.value();

    auto typecheck_result = typecheck(ast, this);
    if (!typecheck_result) {
      printErrorWithSource(typecheck_result.error());
      continue;
    }
    auto &type = typecheck_result.value();

    auto evaluate_result = evaluate(ast, this);
    if (!evaluate_result) {
      printErrorWithSource(evaluate_result.error());
      continue;
    }
    auto &value = evaluate_result.value();

    std::cout << ast << " : " << type << " => " << value << "\n";
  }

  return EXIT_SUCCESS;
}
} // namespace mint
