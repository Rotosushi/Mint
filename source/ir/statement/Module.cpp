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
Module::Module(SourceLocation *sl, Identifier name,
               Expressions expressions) noexcept
    : detail::IrBase(sl), m_name(name), m_expressions(std::move(expressions)),
      m_recovered_expressions(m_expressions.size()) {}
Module::Module(Module const &other) noexcept
    : detail::IrBase(other.sourceLocation()), m_name(other.m_name),
      m_expressions(other.m_expressions),
      m_recovered_expressions(other.m_recovered_expressions) {}
Module::Module(Module &&other) noexcept
    : detail::IrBase(other.sourceLocation()), m_name(other.m_name),
      m_expressions(std::move(other.m_expressions)),
      m_recovered_expressions(std::move(other.m_recovered_expressions)) {}
auto Module::operator=(Module const &other) noexcept -> Module & {
  if (this == &other)
    return *this;

  sourceLocation(other.sourceLocation());
  m_name = other.m_name;
  m_expressions = other.m_expressions;
  m_recovered_expressions = other.m_recovered_expressions;
  return *this;
}
auto Module::operator=(Module &&other) noexcept -> Module & {
  if (this == &other)
    return *this;

  sourceLocation(other.sourceLocation());
  m_name = other.m_name;
  m_expressions = std::move(other.m_expressions);
  m_recovered_expressions = std::move(other.m_recovered_expressions);
  return *this;
}
Module::~Module() noexcept {}

[[nodiscard]] auto Module::name() const noexcept -> Identifier {
  return m_name;
}
[[nodiscard]] auto Module::expressions() noexcept -> Expressions & {
  return m_expressions;
}
[[nodiscard]] auto Module::recovered_expressions() noexcept
    -> Module::Bitset & {
  return m_recovered_expressions;
}
} // namespace ir
} // namespace mint
