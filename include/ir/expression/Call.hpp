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

#include "ir/detail/IrBase.hpp"
#include "ir/detail/Parameter.hpp"

namespace mint {
namespace ir {
class Call : public detail::IrBase {
public:
  using Arguments = std::vector<detail::Parameter>;

private:
  detail::Parameter m_callee;
  Arguments m_arguments;

public:
  Call(SourceLocation *sl, detail::Parameter callee,
       Arguments arguments) noexcept
      : detail::IrBase(sl), m_callee(callee),
        m_arguments(std::move(arguments)) {}
  Call(Call const &other) noexcept = default;
  Call(Call &&other) noexcept = default;
  auto operator=(Call const &other) noexcept -> Call & = default;
  auto operator=(Call &&other) noexcept -> Call & = default;
  ~Call() noexcept = default;

  [[nodiscard]] auto callee() noexcept -> detail::Parameter & {
    return m_callee;
  }
  [[nodiscard]] auto arguments() noexcept -> Arguments & { return m_arguments; }
};
} // namespace ir
} // namespace mint