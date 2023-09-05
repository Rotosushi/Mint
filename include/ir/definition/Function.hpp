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
#include "adt/Attributes.hpp"
#include "adt/Identifier.hpp"
#include "ir/detail/IrBase.hpp"
#include "ir/detail/Parameter.hpp"
#include "type/Type.hpp"

namespace mint::ir {
class Function : public detail::IrBase {
  Attributes m_attributes;
  Identifier m_name;
  FormalArguments m_arguments;
  std::optional<type::Ptr> m_annotation;
  detail::Parameter m_body;

public:
  Function(SourceLocation *sl, Attributes attributes, Identifier name,
           FormalArguments arguments, std::optional<type::Ptr> annotation,
           detail::Parameter body) noexcept
      : detail::IrBase(sl), m_attributes(attributes), m_name(name),
        m_arguments(std::move(arguments)), m_annotation(annotation),
        m_body(body) {}

  [[nodiscard]] auto attributes() const noexcept -> Attributes {
    return m_attributes;
  }
  [[nodiscard]] auto name() const noexcept -> Identifier { return m_name; }
  [[nodiscard]] auto annotation() const noexcept -> std::optional<type::Ptr> {
    return m_annotation;
  }
  [[nodiscard]] auto body() noexcept -> detail::Parameter & { return m_body; }
};
} // namespace mint::ir
