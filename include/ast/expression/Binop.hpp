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

#include "ast/expression/Expression.hpp"
#include "ir/expression/Binop.hpp"
#include "scan/Token.hpp"

namespace mint {
namespace ast {
class Binop : public Expression {
  Token m_op;
  Ptr m_left;
  Ptr m_right;

protected:
  Ptr clone_impl() const noexcept override;
  ir::detail::Parameter flatten_impl(ir::Mir &ir,
                                     bool immediate) const noexcept override;

  // static ir::Binop::Op convert(Token op) noexcept;

public:
  Binop(Attributes attributes, Location location, Token op, Ptr left,
        Ptr right) noexcept;
  ~Binop() noexcept override = default;

  [[nodiscard]] static auto create(Attributes attributes, Location location,
                                   Token op, Ptr left, Ptr right) noexcept
      -> ast::Ptr;

  static auto classof(Ast const *ast) noexcept -> bool;

  void print(std::ostream &out) const noexcept override;

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
