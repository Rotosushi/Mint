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

namespace mint {
namespace ir {
// #TODO: perhaps 'Instruction' isn't the best name?
// the intention is simply the variant representing
// one of the available actions within the IR, that is
// an element of the Array representing a given Ast.
// Node? Element?
// I was sort of planning on using Mir to mean the entire
// flattened Ast. but maybe Mir fits better here? idk
class Instruction {
public:
  using Variant = std::variant<detail::Scalar, Let, Binop, Call, Unop, Import,
                               Module, Lambda>;

private:
  Variant m_variant;

public:
  template <class T, class... Args>
  Instruction(std::in_place_type_t<T> type, Args &&...args) noexcept
      : m_variant(type, std::forward<Args>(args)...) {}
  // #NOTE: since an Instruction is representing a 'flattened'
  // AST node, and there is no way to store pointers consistently
  // within a std::vector. an instruction can only by valid
  // with respect to the  given Mir it is currently
  // residing in. Thus there is no reason why a single
  // instruction should be copied or moved.
  // Only whole vectors can be validly copied or
  // moved. however, to allow for an array to be copied or
  // moved, it's elements must be copyable or movable.
  // so we simply have to accept an un-enforcable invariant
  // to the Mir class.
  Instruction(Instruction const &other) noexcept = default;
  Instruction(Instruction &&other) noexcept = default;
  auto operator=(Instruction const &other) noexcept -> Instruction & = default;
  auto operator=(Instruction &&other) noexcept -> Instruction & = default;
  ~Instruction() noexcept = default;

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }

  [[nodiscard]] auto scalar() -> detail::Scalar & {
    return std::get<detail::Scalar>(m_variant);
  }
  [[nodiscard]] auto let() -> Let & { return std::get<Let>(m_variant); }
  [[nodiscard]] auto binop() -> Binop & { return std::get<Binop>(m_variant); }
  [[nodiscard]] auto call() -> Call & { return std::get<Call>(m_variant); }
  [[nodiscard]] auto unop() -> Unop & { return std::get<Unop>(m_variant); }
  [[nodiscard]] auto import() -> Import & {
    return std::get<Import>(m_variant);
  }
  [[nodiscard]] auto module_() -> Module & {
    return std::get<Module>(m_variant);
  }
  [[nodiscard]] auto lambda() -> Lambda & {
    return std::get<Lambda>(m_variant);
  }
};
} // namespace ir

} // namespace mint
