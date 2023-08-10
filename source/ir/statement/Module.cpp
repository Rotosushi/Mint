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
#include "ir/statement/Module.hpp"
#include "ir/Mir.hpp"

namespace mint {
namespace ir {
Module::Module(Location *sl, Identifier name, Expressions expressions) noexcept
    : detail::Base(sl), m_name(name), m_expressions(std::move(expressions)) {}
Module::Module(Module const &other) noexcept
    : m_name(other.m_name), m_expressions(other.m_expressions) {}
Module::Module(Module &&other) noexcept
    : m_name(other.m_name), m_expressions(std::move(other.m_expressions)) {}
auto Module::operator=(Module const &other) noexcept -> Module & {
  if (this == &other)
    return *this;

  m_name = other.m_name;
  m_expressions = other.m_expressions;
  return *this;
}
auto Module::operator=(Module &&other) noexcept -> Module & {
  if (this == &other)
    return *this;

  m_name = other.m_name;
  m_expressions = std::move(other.m_expressions);
  return *this;
}
Module::~Module() noexcept {}

[[nodiscard]] auto Module::name() const noexcept -> Identifier {
  return m_name;
}
[[nodiscard]] auto Module::expressions() noexcept -> Expressions & {
  return m_expressions;
}
} // namespace ir
} // namespace mint
