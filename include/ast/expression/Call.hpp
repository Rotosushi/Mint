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

#include "ast/AstFwd.hpp"

namespace mint::ast {
struct Call {
  using Arguments = std::vector<Ptr>;
  Ptr callee;
  Arguments arguments;

  Call(Ptr callee, Arguments arguments) noexcept
      : callee(std::move(callee)), arguments(std::move(arguments)) {}
};
} // namespace mint::ast
