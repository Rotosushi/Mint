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
#include "adt/Identifier.hpp"
#include "ir/Mir.hpp"

namespace mint {
namespace ir {
class Module {
  Identifier m_name;
  Mir m_ir;

public:
  Module(Identifier name, Mir ir) noexcept
      : m_name(name), m_ir(std::move(ir)) {}
  Module(Module const &other) noexcept = default;
  Module(Module &&other) noexcept = default;
  auto operator=(Module const &other) noexcept -> Module & = default;
  auto operator=(Module &&other) noexcept -> Module & = default;
  ~Module() noexcept = default;

  [[nodiscard]] auto name() const noexcept -> Identifier { return m_name; }

  [[nodiscard]] auto ir() noexcept -> Mir & { return m_ir; }
  [[nodiscard]] auto ir() const noexcept -> Mir const & { return m_ir; }
};
} // namespace ir
} // namespace mint
