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
// the instention is simply the variant representing
// one of the available actions within the IR, that is
// an element of the Array representing a given Ast.
// Node? Element?
// I was sort of planning on using Mir to mean the entire
// flattened Ast. but maybe Mir fits better here? idk
class Instruction {
public:
  using Variant = std::variant<Let, Binop, Call, Unop, Import, Module, Lambda>;

private:
  Variant m_variant;

public:
  Instruction() noexcept = delete;
  template <class T, class... Args>
  Instruction(std::in_place_type_t<T> type, Args &&...args) noexcept
      : m_variant(type, std::forward<Args>(args)...) {}
  Instruction(Instruction const &other) noexcept = default;
  Instruction(Instruction &&other) noexcept = default;
  auto operator=(Instruction const &other) noexcept -> Instruction & = default;
  auto operator=(Instruction &&other) noexcept -> Instruction & = default;
  ~Instruction() noexcept = default;

  [[nodiscard]] auto variant() noexcept -> Variant & { return m_variant; }
};
} // namespace ir

} // namespace mint
