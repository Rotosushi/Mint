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

#include "adt/Identifier.hpp"
#include "ast/Definition.hpp"

namespace mint {
class LetAst : public DefinitionAst {
  Ptr m_term;

public:
  LetAst(Attributes attributes, Location location,
         std::optional<Type::Ptr> annotation, Identifier name,
         Ptr term) noexcept
      : DefinitionAst{Ast::Kind::Let, attributes, location, annotation, name},
        m_term{std::move(term)} {}

  static auto classof(Ast const *ast) noexcept -> bool {
    return Ast::Kind::Let == ast->kind();
  }

  Ptr clone(Allocator &allocator) const noexcept override {
    return std::allocate_shared<LetAst>(allocator, attributes(), location(),
                                        annotation(), name(), m_term);
  }

  void print(std::ostream &out) const noexcept override {
    out << "let " << name();
    auto anno = annotation();
    if (anno.has_value())
      out << " : " << anno.value();
    out << " = " << m_term;
  }

  Result<Type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<Ast::Ptr> evaluate(Environment &env) noexcept override;
};
} // namespace mint
