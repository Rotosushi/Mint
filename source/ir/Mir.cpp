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
#include "utility/Assert.hpp"

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
Mir::~Mir() noexcept {}
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
void Mir::reserve(std::size_t size) noexcept { m_ir.reserve(size); }

auto Mir::ir() noexcept -> Ir & { return m_ir; }
auto Mir::ir() const noexcept -> Ir const & { return m_ir; }

auto Mir::begin() noexcept -> iterator { return m_ir.begin(); }
auto Mir::end() noexcept -> iterator { return m_ir.end(); }
auto Mir::begin() const noexcept -> const_iterator { return m_ir.begin(); }
auto Mir::end() const noexcept -> const_iterator { return m_ir.end(); }

[[nodiscard]] auto Mir::operator[](std::size_t index) noexcept -> reference {
  MINT_ASSERT(index < size());
  return m_ir[index];
}

[[nodiscard]] auto Mir::operator[](std::size_t index) const noexcept
    -> const_reference {
  MINT_ASSERT(index < size());
  return m_ir[index];
}

template <class T, class... Args>
std::pair<detail::Index, Mir::pointer> Mir::emplace_back(Args &&...args) {
  auto ref =
      m_ir.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
  return {m_index++, &ref};
}

std::pair<detail::Index, Mir::pointer> Mir::emplaceLet(Identifier name,
                                                       Location *sl) {
  return emplace_back<Let>(sl, name);
}

std::pair<detail::Index, Mir::pointer> Mir::emplaceBinop(Binop::Op op,
                                                         Location *sl) {
  return emplace_back<Binop>(sl, op);
}

std::pair<detail::Index, Mir::pointer> Mir::emplaceCall(Location *sl) {
  return emplace_back<Call>(sl);
}

std::pair<detail::Index, Mir::pointer> Mir::emplaceUnop(Unop::Op op,
                                                        Location *sl) {
  return emplace_back<Unop>(sl, op);
}

std::pair<detail::Index, Mir::pointer> Mir::emplaceImport(std::string_view file,
                                                          Location *sl) {
  return emplace_back<Import>(sl, file);
}

std::pair<detail::Index, Mir::pointer>
Mir::emplaceModule(Identifier name, boost::container::vector<Mir> expressions,
                   Location *sl) {
  return emplace_back<Module>(sl, name, std::move(expressions));
}

std::pair<detail::Index, Mir::pointer>
Mir::emplaceLambda(FormalArguments arguments, type::Ptr result_type,
                   Location *sl) {
  return emplace_back<Lambda>(sl, std::move(arguments), result_type);
}
} // namespace ir
} // namespace mint
