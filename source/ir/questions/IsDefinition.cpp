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
#include "ir/questions/IsDefinition.hpp"
#include "ir/Instruction.hpp"

namespace mint {

struct IsDefInstructionVisitor {
  [[nodiscard]] bool operator()(ir::Mir &mir) noexcept {
    return std::visit(*this, mir[mir.root()].variant());
  }

  [[nodiscard]] bool
  operator()([[maybe_unused]] ir::detail::Immediate &immeditate) noexcept {
    return false;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Parens &parens) noexcept {
    return false;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Let &let) noexcept {
    return true;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Binop &binop) noexcept {
    return false;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Unop &unop) noexcept {
    return false;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Import &import) noexcept {
    return true;
  }

  [[nodiscard]] bool operator()([[maybe_unused]] ir::Module &module) noexcept {
    return true;
  }
};

static auto isDef = IsDefInstructionVisitor{};

[[nodiscard]] bool isDefinition(ir::Mir &mir) noexcept { return isDef(mir); }
} // namespace mint