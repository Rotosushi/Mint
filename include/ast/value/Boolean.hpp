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
  Boolean(Attributes attributes, Location location, bool value) noexcept;
  ~Boolean() noexcept override = default;

  [[nodiscard]] static auto create(Attributes attributes, Location location,
                                   bool value) noexcept -> ast::Ptr;

  static auto classof(Ast const *ast) noexcept -> bool;

  auto value() const noexcept -> bool;

  Ptr clone_impl() const noexcept override;
  void flatten(ir::Mir &ir) const noexcept override;
  void
  flatten_immediate(ir::detail::Parameter &parameter) const noexcept override;
  void print(std::ostream &out) const noexcept override;

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
