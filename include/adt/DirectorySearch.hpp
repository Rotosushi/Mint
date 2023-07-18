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
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "utility/Abort.hpp"

namespace fs = std::filesystem;

namespace mint {
class DirectorySearcher {
  std::vector<fs::path> m_known_paths;

  auto existsWithinDirectory(fs::path &directory, fs::path &file) noexcept
      -> bool {
    auto path = directory;
    path /= file;

    std::error_code ec;
    auto result = fs::exists(path, ec);
    if (ec != std::error_code{}) {
      abort(ec);
    }
    return result;
  };

  auto searchWithinDirectory(fs::path &directory, fs::path &file) noexcept
      -> std::optional<std::fstream> {
    auto path = directory;
    path /= file;

    std::fstream file_stream{path};
    if (file_stream.is_open()) {
      return file_stream;
    }

    return std::nullopt;
  }

public:
  DirectorySearcher() noexcept {
    m_known_paths.emplace_back(fs::current_path());
    // #TODO: add the mint standard library path to the space.
  }

  /*
    add another directory to the search space
  */
  void append(fs::path directory) noexcept { m_known_paths.push_back(directory); }

  /*
    check that we can find the given file within
    the search space.
  */
  auto exists(fs::path file) noexcept -> bool {
    for (auto &directory : m_known_paths) {
      if (existsWithinDirectory(directory, file))
        return true;
    }
    return false;
  }

  /*
    attempt to open the file given, checking each
    directory m_known_paths.
  */
  auto search(fs::path file) noexcept -> std::optional<std::fstream> {
    for (auto &directory : m_known_paths) {
      auto found = searchWithinDirectory(directory, file);
      if (found) {
        return found;
      }
    }
    return std::nullopt;
  }
};
} // namespace mint
