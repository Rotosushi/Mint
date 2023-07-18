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

#include "adt/Identifier.hpp"
#include "ast/statement/Statement.hpp"

namespace mint {
namespace ast {
class Module : public Statement {
public:
  using Expressions = std::vector<Ptr>;

private:
  Identifier m_name;
  Expressions m_expressions;

public:
  Module(Attributes attributes, Location location, Identifier name,
         Expressions expressions) noexcept
      : Statement{Ast::Kind::Module, attributes, location}, m_name{name},
        m_expressions{std::move(expressions)} {
    for (auto &expression : m_expressions)
      expression->setPrevAst(this);
  }
  ~Module() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Module;
  }

  Ptr clone(Environment &env) const noexcept override;

  void print(std::ostream &out) const noexcept override {
    out << "module " << m_name << " { \n";

    for (auto &expression : m_expressions)
      out << expression << "\n";

    out << "}";
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
