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
#include <optional>
#include <vector>

#include "adt/Argument.hpp"
#include "ir/detail/IrBase.hpp"
#include "ir/detail/Parameter.hpp"

namespace mint {
namespace ir {
class Lambda : public detail::IrBase {
  // #TODO: lambda's cannot be represented as they are.
  // for one simple reason, as they are they can only
  // be called within the Mir in which they reside.
  // We must be able to store a callable object within
  // the symbol table itself, in the same way we must
  // store scalar values within the symbol table.
  // then call expressions in any given Mir may be able
  // to call the stored lambda.
  // I would say that the body of the lambda must itself be
  // a Mir, however, this would not store the FormalArguments
  // in the symbol table, so how could call expressions work?
  // no, the entire lambda object must be stored. the only
  // way I see that working is by having the symbol table hold
  // a Mir. but then, how do we copy the lambda into that Mir
  // from the Mir where it was defined?
private:
  FormalArguments m_arguments;
  std::optional<type::Ptr> m_annotation;
  detail::Parameter m_body;

public:
  Lambda(SourceLocation *sl, FormalArguments arguments,
         std::optional<type::Ptr> annotation, detail::Parameter body) noexcept
      : detail::IrBase(sl), m_arguments(std::move(arguments)),
        m_annotation(annotation), m_body(body) {}
  Lambda(Lambda const &other) noexcept = default;
  Lambda(Lambda &&other) noexcept = default;
  auto operator=(Lambda const &other) noexcept -> Lambda & = default;
  auto operator=(Lambda &&other) noexcept -> Lambda & = default;
  ~Lambda() noexcept = default;

  [[nodiscard]] auto arguments() const noexcept -> FormalArguments const & {
    return m_arguments;
  }
  [[nodiscard]] auto body() noexcept -> detail::Parameter & { return m_body; }
  [[nodiscard]] auto result_type() const noexcept -> std::optional<type::Ptr> {
    return m_annotation;
  }
};
} // namespace ir
} // namespace mint
