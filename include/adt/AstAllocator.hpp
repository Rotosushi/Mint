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
#include "ast/Equals.hpp"

#include "type/Equals.hpp"

namespace mint {
/*
  I think, since we are performing interpretation,
  we want to clean up unused Ast's more than we want
  to keep everything the user types around for
  the lifetime of the interpreter.
  thus I think it might be a better choice to
  have Ast's be composed out of std::shared_ptrs.
  this allows them to be cleaned up (mostly), when
  they are constructed and not bound in a scope.

  it does raise the question, "when does a
  link in the Ast need to be a weak_ptr?"
  and I don't know the exact answer.
*/
class AstAllocator {
private:
  std::vector<std::unique_ptr<Ast>> resource;

  template <class... Args>
  [[nodiscard]] auto get(Args &&...args) noexcept -> Ast * {
    return resource
        .emplace_back(
            std::unique_ptr<Ast>(new Ast(std::forward<Args>(args)...)))
        .get();
  }

public:
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
