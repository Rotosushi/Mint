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

#include "ast/Print.hpp"

namespace mint {
auto Environment::repl() noexcept -> int {
  bool run = true;

  auto read = [&]() {
    std::string line;
    std::getline(*in, line);
    line.push_back('\n'); // getline doesn't append the newline
    return line;
  };

  while (run) {
    std::cout << "# ";
    auto line = read();
    parser.append(line);

    auto parse_result = parser.parse();
    if (!parse_result)
      printErrorWithSource(parse_result.error());
    else
      std::cout << "> " << parse_result.value() << "\n";
  }

  return EXIT_SUCCESS;
}
} // namespace mint
