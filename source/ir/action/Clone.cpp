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
[[nodiscard]] static detail::Parameter clone(Mir &source, Mir &target,
                                             detail::Index index) noexcept;

struct CloneParameterVisitor {
  Mir &m_source;
  Mir &m_target;

  CloneParameterVisitor(Mir &source, Mir &target) noexcept
      : m_source(source), m_target(target) {}

  detail::Parameter operator()(detail::Parameter &parameter) noexcept {
    return std::visit(*this, parameter.variant());
  }

  detail::Parameter operator()(detail::Immediate &immediate) noexcept {
    return immediate;
  }

  detail::Parameter operator()(detail::Index &index) noexcept {
    return clone(m_source, m_target, index);
  }
};

[[nodiscard]] static detail::Parameter
clone(Mir &source, Mir &target, detail::Parameter &parameter) noexcept {
  CloneParameterVisitor visitor(source, target);
  return visitor(parameter);
}

struct CloneInstructionVisitor {
  Mir &m_source;
  Mir &m_target;

  CloneInstructionVisitor(Mir &source, Mir &target) noexcept
      : m_source(source), m_target(target) {}

  detail::Parameter operator()(detail::Index index) noexcept {
    return std::visit(*this, m_source[index].variant());
  }

  detail::Parameter operator()(detail::Immediate &immediate) noexcept {
    return m_target.emplaceImmediate(immediate);
  }

  detail::Parameter operator()(Affix &affix) noexcept {
    return m_target.emplaceAffix(clone(m_source, m_target, affix.parameter()));
  }

  detail::Parameter operator()(Parens &parens) noexcept {
    return m_target.emplaceParens(
        clone(m_source, m_target, parens.parameter()));
  }

  detail::Parameter operator()(Let &let) noexcept {
    return m_target.emplaceLet(let.name(), let.annotation(),
                               clone(m_source, m_target, let.parameter()));
  }

  detail::Parameter operator()(Binop &binop) noexcept {
    return m_target.emplaceBinop(binop.op(),
                                 clone(m_source, m_target, binop.left()),
                                 clone(m_source, m_target, binop.right()));
  }

  detail::Parameter operator()(Call &call) noexcept {
    auto callee = clone(m_source, m_target, call.callee());

    Call::Arguments arguments;
    arguments.reserve(call.arguments().size());
    for (auto &argument : call.arguments()) {
      arguments.emplace_back(clone(m_source, m_target, argument));
    }

    return m_target.emplaceCall(callee, std::move(arguments));
  }

  detail::Parameter operator()(Unop &unop) noexcept {
    return m_target.emplaceUnop(unop.op(),
                                clone(m_source, m_target, unop.right()));
  }

  detail::Parameter operator()(Import &import) noexcept {
    return m_target.emplaceImport(import.file());
  }

  detail::Parameter operator()(Module &module_) noexcept {
    Module::Expressions expressions;
    expressions.reserve(module_.expressions().size());
    for (auto &expression : module_.expressions()) {
      expressions.emplace_back(clone(expression));
    }

    return m_target.emplaceModule(module_.name(), std::move(expressions));
  }

  detail::Parameter operator()(Lambda &lambda) noexcept {
    return m_target.emplaceLambda(lambda.arguments(), lambda.result_type(),
                                  clone(m_source, m_target, lambda.body()));
  }
};

[[nodiscard]] static detail::Parameter clone(Mir &source, Mir &target,
                                             detail::Index index) noexcept {
  CloneInstructionVisitor visitor(source, target);
  return visitor(index);
}

[[nodiscard]] Mir clone(Mir &ir, detail::Index index) noexcept {
  Mir target;
  [[maybe_unused]] auto p = clone(ir, target, index);
  return target;
}
} // namespace ir
} // namespace mint
