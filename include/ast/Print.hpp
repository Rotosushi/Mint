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
#include <ostream>

#include "ast/Ast.hpp"
#include "type/Print.hpp"

namespace mint {
class AstValuePrintVisitor {
  std::ostream *out;

public:
  AstValuePrintVisitor(std::ostream *out) noexcept : out(out) {
    MINT_ASSERT(out != nullptr);
  }

  void operator()(Ast::Value::Boolean const &boolean) noexcept {
    if (boolean.value)
      *out << "true";
    else
      *out << "false";
  }

  void operator()(Ast::Value::Integer const &integer) noexcept {
    *out << integer.value;
  }

  void operator()([[maybe_unused]] Ast::Value::Nil const &nil) noexcept {
    *out << "nil";
  }
};

inline void print(std::ostream &out, Ast::Value const &value) noexcept {
  AstValuePrintVisitor visitor{&out};
  std::visit(visitor, value.data);
}

inline auto operator<<(std::ostream &out, Ast::Value const &value) noexcept
    -> std::ostream & {
  print(out, value);
  return out;
}

/*
  #TODO: pretty code formating
*/
class AstPrintVisitor {
  std::ostream *out;

public:
  AstPrintVisitor(std::ostream *out) noexcept : out(out) {
    MINT_ASSERT(out != nullptr);
  }

  void operator()(Ast::Type const &type) noexcept { *out << &type.type; }

  void operator()(Ast::Let const &let) noexcept {
    *out << "let " << let.id << " = ";
    std::visit(*this, let.term->data);
  }

  void operator()(Ast::Module const &m) noexcept {
    *out << "module " << m.name << "{ ";

    for (auto &expr : m.expressions) {
      std::visit(*this, expr->data);
    }

    *out << " }";
  }

  void operator()(Ast::Binop const &binop) noexcept {
    std::visit(*this, binop.left->data);
    *out << " " << toString(binop.op) << " ";
    std::visit(*this, binop.right->data);
  }

  void operator()(Ast::Unop const &unop) noexcept {
    *out << toString(unop.op) << " ";
    std::visit(*this, unop.right->data);
  }

  void operator()(Ast::Term const &term) noexcept {
    if (term.ast.has_value())
      std::visit(*this, term.ast.value()->data);

    *out << ";";
  }

  void operator()(Ast::Parens const &parens) noexcept {
    *out << "(";
    std::visit(*this, parens.ast->data);
    *out << ")";
  }

  void operator()(Ast::Value const &value) noexcept { *out << value; }

  void operator()(Ast::Variable const &var) noexcept { *out << var.name; }
};

inline void print(std::ostream &out, Ast const *ast) noexcept {
  AstPrintVisitor visitor{&out};
  std::visit(visitor, ast->data);
}

inline auto operator<<(std::ostream &out, Ast const *ast) noexcept
    -> std::ostream & {
  print(out, ast);
  return out;
}
} // namespace mint
