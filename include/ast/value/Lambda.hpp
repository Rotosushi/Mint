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

#include "ast/value/Value.hpp"

namespace mint {
namespace ast {
class Lambda : public Value {
public:
  struct Argument {
    Identifier name;
    Attributes attributes;
    type::Ptr type;
  };
  using Arguments = std::vector<Argument>;

private:
  Arguments m_arguments;
  type::Ptr m_result_type;
  ast::Ptr m_body;

  static auto getLambdaName(IdentifierSet *set) noexcept -> Identifier;

public:
  Lambda(Attributes attributes, Location location, Arguments arguments,
         type::Ptr result_type, ast::Ptr body) noexcept;
  ~Lambda() noexcept override = default;

  static auto create(Attributes attributes, Location location,
                     Arguments arguments, type::Ptr result_type,
                     ast::Ptr body) noexcept -> ast::Ptr;
  static auto classof(Ast const *ast) noexcept -> bool;

  [[nodiscard]] auto arguments() const noexcept -> Arguments const &;
  [[nodiscard]] auto result_type() const noexcept -> type::Ptr;
  [[nodiscard]] auto body() const noexcept -> ast::Ptr const &;

  Ptr clone() const noexcept override;
  void print(std::ostream &out) const noexcept override;

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint