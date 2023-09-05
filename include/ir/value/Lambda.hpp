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
#include "ir/Mir.hpp"
#include "ir/detail/IrBase.hpp"

// #TODO: handle non-capturing vs. capturing lookup from
// within a lambda.

namespace mint {
namespace ir {

class Lambda : public detail::IrBase {
private:
  FormalArguments m_arguments;
  std::optional<type::Ptr> m_annotation;
  Mir m_body;

public:
  Lambda(SourceLocation *sl, FormalArguments arguments,
         std::optional<type::Ptr> annotation, Mir body) noexcept
      : detail::IrBase(sl), m_arguments(std::move(arguments)),
        m_annotation(annotation), m_body(std::move(body)) {}
  Lambda(Lambda const &other) noexcept = default;
  Lambda(Lambda &&other) noexcept = default;
  auto operator=(Lambda const &other) noexcept -> Lambda & = default;
  auto operator=(Lambda &&other) noexcept -> Lambda & = default;
  ~Lambda() noexcept = default;

  [[nodiscard]] auto arguments() const noexcept -> FormalArguments const & {
    return m_arguments;
  }
  [[nodiscard]] auto annotation() const noexcept -> std::optional<type::Ptr> {
    return m_annotation;
  }
  [[nodiscard]] auto body() noexcept -> Mir & { return m_body; }
};
} // namespace ir
} // namespace mint
