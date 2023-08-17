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
#include "ir/detail/Parameter.hpp"

namespace mint {
namespace ir {
class Lambda {
private:
  FormalArguments m_arguments;
  type::Ptr m_result_type;
  detail::Parameter m_body;

public:
  Lambda(FormalArguments arguments, type::Ptr result_type,
         detail::Parameter body) noexcept
      : m_arguments(std::move(arguments)), m_result_type(result_type),
        m_body(body) {}
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
    return m_result_type == nullptr ? std::optional<type::Ptr>{}
                                    : std::optional<type::Ptr>{m_result_type};
  }
};
} // namespace ir
} // namespace mint
