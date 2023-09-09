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
#include "comptime/Parse.hpp"
#include "adt/Environment.hpp"
#include "ir/questions/IsDefinition.hpp"

namespace mint {
int parse(fs::path path, Environment &env) noexcept {
  auto found = env.fileSearch(path);
  if (!found) {
    return EXIT_FAILURE;
  }
  auto &file = found.value();
  env.pushActiveSourceFile(std::move(file));

  while (true) {
    auto result = env.parseMir();
    if (!result) {
      auto error = result.error();
      if (error.kind() == Error::Kind::EndOfInput) {
        break;
      }

      env.errorStream() << error << "\n";
      return EXIT_FAILURE;
    }

    if (ir::isDefinition(result.value())) {
      env.addLocalExpression(std::move(result.value()));
    }
  }

  env.popActiveSourceFile();
  return EXIT_SUCCESS;
}
} // namespace mint
