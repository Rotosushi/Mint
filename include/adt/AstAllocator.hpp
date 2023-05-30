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
#include <memory> // std::unique_ptr

#include "ast/Ast.hpp"

#include "utility/Allocator.hpp"

namespace mint {
/*
  do we enforce uniqueness?

  pros:
    ) we gain memory efficiency by not allocating
      an ast if we already have one allocated.
    ) we can implement ast equality via pointer equality.
  cons:
    ) we lose time efficiency by checking if we have already
      allocated this particular ast each time we construct
      a new ast.
*/

class AstAllocator {
public:
  Allocator *allocator;
  std::vector<ManagedPointer<Ast>> buffer;

  template <class... Args>
  [[nodiscard]] auto get(Args &&...args) noexcept -> Ast * {
    return buffer
        .emplace_back(allocator->construct<Ast>(std::forward<Args>(args)...))
        .get();
  }

public:
  AstAllocator(Allocator *allocator) noexcept : allocator(allocator) {
    MINT_ASSERT(allocator != nullptr);
  }

  auto getAffix(Location location, Ast *affix) noexcept {
    return get(std::in_place_type<Ast::Term>, location, affix);
  }

  auto getType(Location location, mint::Type::Pointer type) noexcept {
    return get(std::in_place_type<Ast::Type>, location, type);
  }

  auto getLet(Location location, Identifier name, Ast *term) noexcept {
    return get(std::in_place_type<Ast::Let>, location, name, term);
  }

  auto getBinop(Location location, Token op, Ast *left, Ast *right) noexcept {
    return get(std::in_place_type<Ast::Binop>, location, op, left, right);
  }

  auto getUnop(Location location, Token op, Ast *right) noexcept {
    return get(std::in_place_type<Ast::Unop>, location, op, right);
  }

  auto getParens(Location location, Ast *ast) noexcept {
    return get(std::in_place_type<Ast::Parens>, location, ast);
  }

  auto getVariable(Location location, Identifier name) noexcept {
    return get(std::in_place_type<Ast::Variable>, location, name);
  }

  auto getBoolean(Location location, bool value) noexcept {
    return get(std::in_place_type<Ast::Value>,
               std::in_place_type<Ast::Value::Boolean>, location, value);
  }

  auto getInteger(Location location, int value) noexcept {
    return get(std::in_place_type<Ast::Value>,
               std::in_place_type<Ast::Value::Integer>, location, value);
  }

  auto getNil(Location location) noexcept {
    return get(std::in_place_type<Ast::Value>,
               std::in_place_type<Ast::Value::Nil>, location);
  }
};
} // namespace mint
