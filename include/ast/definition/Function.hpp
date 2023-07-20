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
#include <vector>

#include "ast/definition/Definition.hpp"

namespace mint {
namespace ast {
/*
class Function : public Definition {
public:
using Argument = std::pair<Identifier, type::Ptr>;
using Arguments = std::vector<Argument>;

private:
Arguments m_arguments;
Ptr m_body;

public:
Function(Attributes attributes, Location location,
         std::optional<type::Ptr> annotation, Identifier name,
         Arguments arguments, Ptr body) noexcept
    : Definition{Ast::Kind::Function, attributes, location, annotation, name},
      m_arguments{std::move(arguments)}, m_body{std::move(body)} {
  m_body->setPrevAst(this);
}
};
*/
} // namespace ast
} // namespace mint
