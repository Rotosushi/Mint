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
#include "ast/value/Integer.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Result<type::Ptr> Integer::typecheck(Environment &env) const noexcept {
  return env.getIntegerType();
}

Result<ast::Ptr> Integer::evaluate([[maybe_unused]] Environment &env) noexcept {
  return shared_from_this();
}
} // namespace ast
} // namespace mint