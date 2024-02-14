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
std::optional<std::reference_wrapper<ImportedTranslationUnit>>
importSourceFile(fs::path path, Environment &env) noexcept {
  auto found = env.fileSearch(path);
  MINT_ASSERT(found);
  auto &pair = found.value();
  auto &file = pair.first;
  auto &found_path = pair.second;

  if (env.alreadyImported(found_path)) {
    return *env.findImport(found_path);
  }

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
      return std::nullopt;
    }

    if (ast::hasSideEffect(result.value())) {
      expressions.emplace_back(std::move(result.value()));
    }
  }

  env.popActiveSourceFile();
  return env.addImport(std::move(found_path), std::move(expressions));
}
} // namespace mint
