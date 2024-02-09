// Copyright (C) 2024 Cade Weinberg
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
#include <list>
#include <mutex>

namespace fs = std::filesystem;

#include "adt/SourceFile.hpp"

namespace mint {
// Represents the set of files which must be linked together to
// produce the final executable (or eventually library)
class SourceFiles {
private:
  std::mutex m_files_mutex;
  std::list<SourceFile> m_unique_files;

public:
  void add(fs::path path) {
    std::unique_lock<std::mutex> lock(m_files_mutex);

    for (SourceFile &file : m_unique_files) {
      if (file.path() == path) {
        return;
      }
    }

    m_unique_files.emplace_back(std::move(path));
  }

  auto begin() { return m_unique_files.begin(); }
  auto end() { return m_unique_files.end(); }
};
} // namespace mint
