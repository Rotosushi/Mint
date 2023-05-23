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

#include "type/Equals.hpp"

namespace mint {
struct AstValueEqualsVisitor {
  Ast::Value const *left;
  Ast::Value const *right;

  AstValueEqualsVisitor(Ast::Value const *left,
                        Ast::Value const *right) noexcept
      : left(left), right(right) {}

  auto operator()() noexcept -> bool { return std::visit(*this, left->data); }

  auto operator()(Ast::Value::Boolean const &left_boolean) noexcept -> bool {
    auto right_boolean = std::get_if<Ast::Value::Boolean>(&right->data);
    if (right_boolean == nullptr)
      return false;

    return left_boolean.value == right_boolean->value;
  }

  auto operator()(Ast::Value::Integer const &left_integer) noexcept -> bool {
    auto right_integer = std::get_if<Ast::Value::Integer>(&right->data);
    if (right_integer == nullptr)
      return false;

    return left_integer.value == right_integer->value;
  }

  /*
   nil == nil
  */
  auto operator()([[maybe_unused]] Ast::Value::Nil const &left_nil) noexcept
      -> bool {
    return std::holds_alternative<Ast::Value::Nil>(right->data);
  }
};

[[nodiscard]] inline auto equals(Ast::Value const *left,
                                 Ast::Value const *right) noexcept {
  AstValueEqualsVisitor visitor{left, right};
  return visitor();
}

[[nodiscard]] inline auto equals(Ast const *left, Ast const *right) noexcept
    -> bool;

struct AstEqualsVisitor {
  Ast const *left;
  Ast const *right;

  AstEqualsVisitor(Ast const *left, Ast const *right) noexcept
      : left(left), right(right) {}

  auto operator()() noexcept -> bool { return std::visit(*this, left->data); }

  auto operator()(Ast::Affix const &left_affix) noexcept -> bool {
    auto right_affix = std::get_if<Ast::Affix>(&right->data);
    if (right_affix == nullptr)
      return false;

    return equals(left_affix.affix, right_affix->affix);
  }

  auto operator()(Ast::Type const &left_type) noexcept -> bool {
    auto right_type = std::get_if<Ast::Type>(&right->data);
    if (right_type == nullptr)
      return false;

    return equals(left_type.type, right_type->type);
  }

  auto operator()(Ast::Let const &left_let) noexcept -> bool {
    auto right_let = std::get_if<Ast::Let>(&right->data);
    if (right_let == nullptr)
      return false;

    if (left_let.id != right_let->id)
      return false;

    return equals(left_let.term, right_let->term);
  }

  auto operator()(Ast::Binop const &left_binop) noexcept -> bool {
    auto right_binop = std::get_if<Ast::Binop>(&right->data);
    if (right_binop == nullptr)
      return false;

    if (!equals(left_binop.left, right_binop->left))
      return false;

    if (left_binop.op != right_binop->op)
      return false;

    return equals(left_binop.right, right_binop->right);
  }

  auto operator()(Ast::Unop const &left_unop) noexcept -> bool {
    auto right_unop = std::get_if<Ast::Unop>(&right->data);
    if (right_unop == nullptr)
      return false;

    if (left_unop.op != right_unop->op)
      return false;

    return equals(left_unop.right, right_unop->right);
  }

  auto operator()(Ast::Value const &left_value) noexcept -> bool {
    auto right_value = std::get_if<Ast::Value>(&right->data);
    if (right_value == nullptr)
      return false;

    return equals(&left_value, right_value);
  }

  auto operator()(Ast::Parens const &left_parens) noexcept -> bool {
    auto right_parens = std::get_if<Ast::Parens>(&right->data);
    if (right_parens == nullptr)
      return false;

    return equals(left_parens.ast, right_parens->ast);
  }

  auto operator()(Ast::Variable const &left_variable) noexcept -> bool {
    auto right_variable = std::get_if<Ast::Variable>(&right->data);
    if (right_variable == nullptr)
      return false;

    return left_variable.name == right_variable->name;
  }
};

[[nodiscard]] inline auto equals(Ast const *left, Ast const *right) noexcept
    -> bool {
  AstEqualsVisitor visitor(left, right);
  return visitor();
}
} // namespace mint
