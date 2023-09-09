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
#include <bitset>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

#include "adt/TranslationUnit.hpp"

namespace mint {
class ImportedTranslationUnit {
  class Context {
    enum Flags {
      Evaluated,
      Generated,
      SIZE,
    };
    std::bitset<SIZE> m_set;

  public:
    [[nodiscard]] bool evaluated() const noexcept { return m_set[Evaluated]; }
    bool evaluated(bool state) noexcept { return m_set[Evaluated] = state; }
    [[nodiscard]] bool generated() const noexcept { return m_set[Generated]; }
    bool generated(bool state) noexcept { return m_set[Generated] = state; }
  };

  Context m_context;
  fs::path m_file;
  TranslationUnit m_translation_unit;

public:
  ImportedTranslationUnit(fs::path &&file,
                          TranslationUnit::Expressions &&expressions) noexcept
      : m_file(std::move(file)), m_translation_unit(std::move(expressions)) {}

  Context &context() { return m_context; }

  fs::path const &file() const noexcept { return m_file; }

  void append(ir::Mir &&mir) noexcept {
    m_translation_unit.append(std::move(mir));
  }

  TranslationUnit::Expressions &expressions() noexcept {
    return m_translation_unit.m_expressions;
  }
  TranslationUnit::Bitset &recovered_expressions() noexcept {
    return m_translation_unit.m_recovered_expressions;
  }
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

  ImportedTranslationUnit &
  insert(fs::path &&filename,
         TranslationUnit::Expressions &&expressions) noexcept {
    if (auto itu = find(filename); itu != nullptr) {
      return *itu;
    }
    return m_set.emplace_back(std::move(filename), std::move(expressions));
  }
};
} // namespace mint
