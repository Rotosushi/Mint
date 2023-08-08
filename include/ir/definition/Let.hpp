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
#include "ir/detail/Parameter.hpp"

namespace mint {
namespace ir {
class Let {
  Identifier m_name;
  detail::Parameter m_parameter;

public:
  Let(Identifier name) noexcept : m_name(name) {}
  Let(Let const &other) noexcept = default;
  Let(Let &&other) noexcept = default;
  auto operator=(Let const &other) noexcept -> Let & = default;
  auto operator=(Let &&other) noexcept -> Let & = default;
  ~Let() noexcept = default;

  [[nodiscard]] auto name() const noexcept -> Identifier { return m_name; }
  [[nodiscard]] auto parameter() noexcept -> detail::Parameter & {
    return m_parameter;
  }
};
} // namespace ir
} // namespace mint
