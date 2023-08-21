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
#include "ir/All.hpp"
#include "type/Type.hpp"

namespace mint {
namespace ir {
// #NOTE: since an Instruction is representing a 'flattened'
// AST node, and there is no way to store pointers consistently
// within a std::vector. an instruction can only by valid
// with respect to the given Mir it is currently
// residing in. Thus there is no reason why a single
// instruction should be copied or moved.
// Only whole vectors can be validly copied or
// moved. however, to allow for an array to be copied or
// moved, it's elements must be copyable or movable.
// so we simply have to accept an un-enforcable invariant
// to the Mir class.

// represents a flattened node within the AST
class Instruction {
public:
  using Variant = std::variant<detail::Immediate, Affix, Parens, Let, Binop,
                               Unop, Import, Module, Call, Lambda>;

private:
  type::Ptr m_cached_type;
  Variant m_variant;

public:
  template <class T, class... Args>
  Instruction(std::in_place_type_t<T> type, Args &&...args) noexcept
      : m_cached_type(nullptr), m_variant(type, std::forward<Args>(args)...) {}
  Instruction(Instruction const &other) noexcept = default;
  Instruction(Instruction &&other) noexcept = default;
  auto operator=(Instruction const &other) noexcept -> Instruction & = default;
  auto operator=(Instruction &&other) noexcept -> Instruction & = default;
  ~Instruction() noexcept = default;

  [[nodiscard]] type::Ptr cachedType() const noexcept { return m_cached_type; }
  type::Ptr cachedType(type::Ptr type) noexcept { return m_cached_type = type; }

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }
};
} // namespace ir

} // namespace mint
