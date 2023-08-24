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
Mir::Mir() noexcept : m_index(), m_ir() {}
Mir::Mir(Mir const &other) noexcept
    : m_index(other.m_index), m_ir(other.m_ir) {}
Mir::Mir(Mir &&other) noexcept
    : m_index(other.m_index), m_ir(std::move(other.m_ir)) {}
Mir::~Mir() noexcept {}
auto Mir::operator=(Mir const &other) noexcept -> Mir & {
  if (this == &other)
    return *this;

  m_index = other.m_index;
  m_ir = other.m_ir;
  return *this;
}
auto Mir::operator=(Mir &&other) noexcept -> Mir & {
  if (this == &other)
    return *this;

  m_index = other.m_index;
  m_ir = std::move(other.m_ir);
  return *this;
}

Mir::operator bool() const noexcept { return !empty(); }
auto Mir::empty() const noexcept -> bool { return m_ir.empty(); }
auto Mir::size() const noexcept -> std::size_t { return m_ir.size(); }
void Mir::reserve(std::size_t size) noexcept { m_ir.reserve(size); }

auto Mir::root() const noexcept -> detail::Index {
  if (m_index == 0)
    return 0;

  return m_index - 1;
}
auto Mir::ir() noexcept -> Ir & { return m_ir; }
auto Mir::ir() const noexcept -> Ir const & { return m_ir; }

[[nodiscard]] auto Mir::operator[](detail::Index index) noexcept -> reference {
  MINT_ASSERT(index < size());
  return m_ir[index.index()];
}

[[nodiscard]] auto Mir::operator[](detail::Index index) const noexcept
    -> const_reference {
  MINT_ASSERT(index < size());
  return m_ir[index.index()];
}

// #NOTE: insert instruction at current index,
// increment index to next open slot.
template <class T, class... Args>
detail::Index Mir::emplace_back(Args &&...args) {
  m_ir.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
  return m_index++;
}

detail::Index Mir::emplaceAffix(SourceLocation *sl,
                                detail::Parameter parameter) {
  return emplace_back<Affix>(sl, parameter);
}

detail::Index Mir::emplaceParens(SourceLocation *sl,
                                 detail::Parameter parameter) {
  return emplace_back<Parens>(sl, parameter);
}

detail::Index Mir::emplaceLet(SourceLocation *sl, Identifier name,
                              std::optional<type::Ptr> annotation,
                              detail::Parameter parameter) {
  return emplace_back<Let>(sl, name, annotation, parameter);
}

detail::Index Mir::emplaceBinop(SourceLocation *sl, Token op,
                                detail::Parameter left,
                                detail::Parameter right) {
  return emplace_back<Binop>(sl, op, left, right);
}

detail::Index Mir::emplaceUnop(SourceLocation *sl, Token op,
                               detail::Parameter right) {
  return emplace_back<Unop>(sl, op, right);
}

detail::Index Mir::emplaceImport(SourceLocation *sl, std::string_view file) {
  return emplace_back<Import>(sl, file);
}

detail::Index Mir::emplaceModule(SourceLocation *sl, Identifier name,
                                 boost::container::vector<Mir> expressions) {
  return emplace_back<Module>(sl, name, std::move(expressions));
}

detail::Index Mir::emplaceCall(SourceLocation *sl, detail::Parameter callee,
                               std::vector<detail::Parameter> arguments) {
  return emplace_back<Call>(sl, callee, std::move(arguments));
}

detail::Index Mir::emplaceLambda(SourceLocation *sl, FormalArguments arguments,
                                 std::optional<type::Ptr> annotation,
                                 Mir body) {
  return emplace_back<Lambda>(sl, std::move(arguments), annotation, body);
}
} // namespace ir
} // namespace mint
