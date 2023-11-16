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
#include "ir/definition/Function.hpp"

namespace mint::ir {
Function::Function(SourceLocation *sl, Attributes attributes, Identifier name,
                   FormalArguments arguments,
                   std::optional<type::Ptr> annotation, Body body) noexcept
    : detail::IrBase(sl), m_attributes(attributes), m_name(name),
      m_arguments(std::move(arguments)), m_annotation(annotation),
      m_body(std::move(body)) {}

[[nodiscard]] auto Function::attributes() noexcept -> Attributes & {
  return m_attributes;
}
[[nodiscard]] auto Function::name() const noexcept -> Identifier {
  return m_name;
}
[[nodiscard]] auto Function::arguments() noexcept -> FormalArguments & {
  return m_arguments;
}
[[nodiscard]] auto Function::annotation() const noexcept
    -> std::optional<type::Ptr> {
  return m_annotation;
}
[[nodiscard]] auto Function::body() noexcept -> Body & { return m_body; }
} // namespace mint::ir
