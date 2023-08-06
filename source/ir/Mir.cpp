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
#include "ir/Mir.hpp"
#include "ir/Instruction.hpp"

namespace mint {
namespace ir {
// #NOTE: all of these functions -must- be out of line
// because Instruction cannot be a full type at the time
// that Mir is defined, because any IR Instruction which
// is composed of a series of Instructions itself, (Module, Block)
// is composed of a Mir.
Mir::Mir() noexcept : m_ir() {}
Mir::Mir(Ir ir) noexcept : m_ir(std::move(ir)) {}
Mir::Mir(Mir const &other) noexcept : m_ir(other.m_ir) {}
Mir::Mir(Mir &&other) noexcept : m_ir(std::move(other.m_ir)) {}
auto Mir::operator=(Mir const &other) noexcept -> Mir & {
  if (this == &other)
    return *this;

  m_ir = other.m_ir;
  return *this;
}
auto Mir::operator=(Mir &&other) noexcept -> Mir & {
  if (this == &other)
    return *this;

  m_ir = std::move(other.m_ir);
  return *this;
}

auto Mir::empty() const noexcept -> bool { return m_ir.empty(); }
auto Mir::size() const noexcept -> std::size_t { return m_ir.size(); }
auto Mir::ir() noexcept -> Ir & { return m_ir; }
auto Mir::ir() const noexcept -> Ir const & { return m_ir; }

auto Mir::begin() noexcept -> iterator { return m_ir.begin(); }
auto Mir::end() noexcept -> iterator { return m_ir.end(); }
auto Mir::begin() const noexcept -> const_iterator { return m_ir.begin(); }
auto Mir::end() const noexcept -> const_iterator { return m_ir.end(); }

template <class... Args> Mir::reference Mir::emplace_back(Args &&...args) {
  return m_ir.emplace_back(std::forward<Args>(args)...);
}
} // namespace ir
} // namespace mint
