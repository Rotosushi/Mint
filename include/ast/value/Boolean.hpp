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

#include "ast/value/Value.hpp"

namespace mint {
namespace ast {
class Boolean : public Value {
  bool m_value;

public:
  Boolean(Attributes attributes, Location location, bool value) noexcept
      : Value{Ast::Kind::Boolean, attributes, location}, m_value{value} {}
  ~Boolean() noexcept override = default;

  auto value() const noexcept { return m_value; }

  static auto create(Attributes attributes, Location location,
                     bool value) noexcept -> Ptr {
    return std::make_shared<Boolean>(attributes, location, value);
  }

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Boolean;
  }

  Ptr clone() const noexcept override {
    return create(attributes(), location(), m_value);
  }

  void print(std::ostream &out) const noexcept override {
    out << (m_value ? "true" : "false");
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
