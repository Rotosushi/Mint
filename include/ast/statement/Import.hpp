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

#include "ast/statement/Statement.hpp"

namespace mint {
namespace ast {
class Import : public Statement {
  std::string m_filename;

public:
  Import(Attributes attributes, Location location,
         std::string filename) noexcept
      : Statement{Ast::Kind::Import, attributes, location},
        m_filename{std::move(filename)} {}
  ~Import() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return ast->kind() == Ast::Kind::Import;
  }

  Ptr clone(Environment &env) const noexcept override;

  void print(std::ostream &out) const noexcept override {
    out << "import " << m_filename << ";";
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
