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
#include "adt/DirectorySearch.hpp"

namespace mint {
auto DirectorySearcher::existsWithinDirectory(fs::path const &directory,
                                              fs::path const &file) noexcept
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

auto DirectorySearcher::searchWithinDirectory(fs::path const &directory,
                                              fs::path const &file) noexcept
    -> std::optional<std::fstream> {
  auto path = directory;
  path /= file;

  std::fstream file_stream{path, std::ios_base::in};
  if (file_stream.is_open()) {
    return file_stream;
  }

  return std::nullopt;
}

void DirectorySearcher::append(fs::path const &directory) noexcept {
  m_known_paths.emplace_back(std::move(directory));
}

auto DirectorySearcher::exists(fs::path const &file) noexcept -> bool {
  for (auto &directory : m_known_paths) {
    if (existsWithinDirectory(directory, file))
      return true;
  }
  return false;
}

auto DirectorySearcher::search(fs::path const &file) noexcept
    -> std::optional<std::fstream> {
  for (auto &directory : m_known_paths) {
    auto found = searchWithinDirectory(directory, file);
    if (found) {
      return found;
    }
  }
  return std::nullopt;
}
} // namespace mint
