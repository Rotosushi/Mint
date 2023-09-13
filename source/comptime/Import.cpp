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
#include "comptime/Import.hpp"
#include "adt/Environment.hpp"
#include "ast/visitors/HasSideEffect.hpp"
#include "comptime/Evaluate.hpp"
#include "comptime/Parse.hpp"
#include "comptime/Typecheck.hpp"

namespace mint {
// #TODO: maybe not the best name
int importSourceFile(fs::path path, Environment &env) noexcept {
  if (env.alreadyImported(path)) {
    return EXIT_SUCCESS;
  }

  auto found = env.fileSearch(path);
  MINT_ASSERT(found);
  auto &file = found.value();

  env.pushActiveSourceFile(std::move(file));
  TranslationUnit::Expressions expressions;

  while (true) {
    auto result = env.parse();
    if (!result) {
      auto error = result.error();
      if (error.kind() == Error::Kind::EndOfInput) {
        break;
      }

      env.errorStream() << error << "\n";
      return EXIT_FAILURE;
    }

    if (ast::hasSideEffect(result.value())) {
      expressions.emplace_back(std::move(result.value()));
    }
  }

  env.popActiveSourceFile();
  env.addImport(std::move(path), std::move(expressions));
  return EXIT_SUCCESS;
}
} // namespace mint
