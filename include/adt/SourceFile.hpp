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
#include <mutex>

namespace fs = std::filesystem;

class SourceFile {
public:
  enum State {
    Unprocessed,
    InProgress,
    Done,
    Failed,
  };

private:
  fs::path m_path;
  State m_state;
  std::mutex m_state_mutex;

public:
  SourceFile(fs::path path) : m_path(std::move(path)), m_state(Unprocessed) {}

  fs::path const &path() const noexcept { return m_path; }

  State state() noexcept {
    std::unique_lock lock(m_state_mutex);
    return m_state;
  }

  State state(State new_state) noexcept {
    std::unique_lock lock(m_state_mutex);
    return m_state = new_state;
  }

  bool unprocessed() noexcept { return state() == Unprocessed; }
  bool in_progress() noexcept { return state() == InProgress; }
  bool done() noexcept { return state() == Done; }
  bool failed() noexcept { return state() == Failed; }
};