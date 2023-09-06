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
#include <vector>

#include "ir/Mir.hpp"
#include "adt/ImportSet.hpp"

namespace mint {
class TranslationUnit {
public:
  using Expression = ir::Mir;
  using Expressions = std::vector<ir::Mir>;

private:
  Expressions m_local;
  Expressions m_imported;

public:
  void addLocalExpression(TranslationUnit::Expression &&expression) noexcept {
    m_local.emplace_back(std::move(expression));
  }

  void
  addImportedExpression(TranslationUnit::Expression &&expression) noexcept {
    m_imported.emplace_back(std::move(expression));
  }
  TranslationUnit::Expressions &localExpressions() noexcept { return m_local; }
  TranslationUnit::Expressions &importedExpressions() noexcept {
    return m_imported;
  }
};
} // namespace mint
