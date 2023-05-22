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
#include "ast/Ast.hpp"
#include "ast/Equals.hpp"

#include "type/Equals.hpp"

namespace mint {
class AstAllocator {
private:
  // at some point we can be clever about allocating Asts
  class Set {
    std::vector<std::unique_ptr<Ast>> set;

  public:
    template <class... Args>
    auto get(Location location, Args &&...args) noexcept -> Ast * {
      Ast possible{location, std::forward<Args>(args)...};

      for (auto &ast : set) {
        if (equals(ast.get(), &possible)) {
          return ast.get();
        }
      }

      auto &new_element =
          set.emplace_back(std::unique_ptr<Ast>(new Ast(std::move(possible))));
      return new_element.get();
    }
  };

  Set types;
  Set lets;
  Set binops;
  Set unops;
  Set integers;
  Set booleans;
  Set nils;
  Set parens;
  Set variables;

public:
  auto getType(Location location, mint::Type::Pointer type) noexcept {
    return types.get(location, std::in_place_type<Ast::Type>, type);
  }

  auto getLet(Location location, Identifier name, Ast *term) noexcept {
    return lets.get(location, std::in_place_type<Ast::Let>, name, term);
  }

  auto getBinop(Location location, Token op, Ast *left, Ast *right) noexcept {
    return binops.get(location, std::in_place_type<Ast::Binop>, op, left,
                      right);
  }

  auto getUnop(Location location, Token op, Ast *right) noexcept {
    return unops.get(location, std::in_place_type<Ast::Unop>, op, right);
  }

  auto getBoolean(Location location, bool value) noexcept {
    return booleans.get(location, std::in_place_type<Ast::Value>,
                        std::in_place_type<Ast::Value::Boolean>, value);
  }

  auto getInteger(Location location, int value) noexcept {
    return integers.get(location, std::in_place_type<Ast::Value>,
                        std::in_place_type<Ast::Value::Integer>, value);
  }

  auto getNil(Location location) noexcept {
    return nils.get(location, std::in_place_type<Ast::Value>,
                    std::in_place_type<Ast::Value::Nil>);
  }

  auto getParens(Location location, Ast *ast) noexcept {
    return parens.get(location, std::in_place_type<Ast::Parens>, ast);
  }

  auto getVariable(Location location, Identifier name) noexcept {
    return variables.get(location, std::in_place_type<Ast::Variable>, name);
  }
};
} // namespace mint
