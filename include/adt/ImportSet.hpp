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
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

#include "ir/Mir.hpp"

namespace mint {
class ImportedTranslationUnit {
  fs::path m_file;
  std::vector<ir::Mir> m_expressions;

public:
  ImportedTranslationUnit(fs::path &&file,
                          std::vector<ir::Mir> &&expressions) noexcept
      : m_file(std::move(file)), m_expressions(std::move(expressions)) {}

  fs::path const &file() const noexcept { return m_file; }

  void append(ir::Mir &&mir) noexcept {
    m_expressions.emplace_back(std::move(mir));
  }

  std::vector<ir::Mir> &expressions() noexcept { return m_expressions; }
};

class ImportSet {
  std::vector<ImportedTranslationUnit> m_set;

public:
  [[nodiscard]] auto contains(fs::path const &filename) noexcept -> bool {
    for (auto &itu : m_set) {
      if (itu.file() == filename) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] ImportedTranslationUnit *
  find(fs::path const &filename) noexcept {
    for (auto &itu : m_set) {
      if (itu.file() == filename) {
        return &itu;
      }
    }
    return nullptr;
  }

  ImportedTranslationUnit &insert(fs::path &&filename,
                                  std::vector<ir::Mir> &&expressions) noexcept {
    if (!contains(filename)) {
      return m_set.emplace_back(std::move(filename), std::move(expressions));
    }
  }
};
} // namespace mint
