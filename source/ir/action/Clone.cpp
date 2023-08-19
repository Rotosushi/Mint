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
#include "ir/action/Clone.hpp"
#include "ir/Instruction.hpp"

namespace mint {
namespace ir {
struct cloneParameterVisitor {
  Mir &m_source;
  Mir &m_target;

};

[[nodiscard]] static void clone(Mir &source, Mir &target,
                                detail::Parameter &parameter) noexcept {
  cloneParameterVisitor visitor(source, target);
  visitor(parameter);
}

struct cloneInstructionVisitor {
  Mir &m_source;
  Mir &m_target;

  cloneInstructionVisitor(Mir &source, Mir &target) noexcept
      : m_source(source), m_target(target) {}

  void operator()(detail::Index index) noexcept {
    std::visit(*this, m_source[index].variant());
  }

  void operator()(detail::Immediate &immediate) noexcept {
    m_target.emplaceImmediate(immediate);
  }

  void operator()(Let &let) noexcept { m_target.emplaceLet(let.name(), ) }

  void operator()(Binop &binop) noexcept {}
  void operator()(Call &call) noexcept {}
  void operator()(Unop &unop) noexcept {}
  void operator()(Import &import) noexcept {}
  void operator()(Module &module_) noexcept {}
  void operator()(Lambda &lambda) noexcept {}
};

[[nodiscard]] Mir clone(Mir &ir, detail::Index index) noexcept {
  Mir target;
  cloneInstructionVisitor visitor(ir, target);
  visitor(index);
  return target;
}
} // namespace ir
} // namespace mint
