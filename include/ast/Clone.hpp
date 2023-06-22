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

#include "adt/Environment.hpp"

namespace mint {

struct AstValueCloneVisitor {
  Environment *env;

  AstValueCloneVisitor(Environment *env) noexcept : env(env) {
    MINT_ASSERT(env != nullptr);
  }

  auto operator()(Ast::Value const &value) const noexcept -> Ast::Ptr {
    return std::visit(*this, value.data);
  }

  auto operator()(Ast::Value::Boolean const &boolean) const noexcept
      -> Ast::Ptr {
    return env->getBooleanAst(boolean.attributes, boolean.location,
                              boolean.value);
  }

  auto operator()(Ast::Value::Integer const &integer) const noexcept
      -> Ast::Ptr {
    return env->getIntegerAst(integer.attributes, integer.location,
                              integer.value);
  }

  auto operator()(Ast::Value::Nil const &nil) const noexcept -> Ast::Ptr {
    return env->getNilAst(nil.attributes, nil.location);
  }
};

[[nodiscard]] inline auto clone(Ast::Value const &value,
                                Environment *env) noexcept {
  AstValueCloneVisitor visitor{env};
  return visitor(value);
}

struct AstCloneVisitor {
  Environment *env;

  AstCloneVisitor(Environment *env) noexcept : env(env) {
    MINT_ASSERT(env != nullptr);
  }

  auto operator()(Ast::Ptr const &ast) noexcept -> Ast::Ptr {
    return std::visit(*this, ast->data);
  }

  auto operator()(Ast::Type const &type) noexcept -> Ast::Ptr {
    return env->getTypeAst(type.attributes, type.location, type.type);
  }

  auto operator()(Ast::Let const &let) noexcept -> Ast::Ptr {
    auto term = (*this)(let.term);
    return env->getLetAst(let.attributes, let.location, let.id, term);
  }

  auto operator()(Ast::Module const &m) noexcept -> Ast::Ptr {
    std::vector<Ast::Ptr> expressions;
    for (auto &expr : m.expressions) {
      expressions.emplace_back((*this)(expr));
    }
    return env->getModuleAst(m.attributes, m.location, m.name,
                             std::move(expressions));
  }

  auto operator()(Ast::Import const &i) noexcept -> Ast::Ptr {
    return env->getImportAst(i.attributes, i.location, i.file);
  }

  auto operator()(Ast::Binop const &binop) noexcept -> Ast::Ptr {
    auto left = (*this)(binop.left);
    auto right = (*this)(binop.right);
    return env->getBinopAst(binop.attributes, binop.location, binop.op,
                            std::move(left), std::move(right));
  }

  auto operator()(Ast::Unop const &unop) noexcept -> Ast::Ptr {
    auto right = (*this)(unop.right);
    return env->getUnopAst(unop.attributes, unop.location, unop.op,
                           std::move(right));
  }

  auto operator()(Ast::Term const &term) noexcept -> Ast::Ptr {
    std::optional<Ast::Ptr> ast;
    if (term.ast.has_value())
      ast = (*this)(term.ast.value());
    return env->getTermAst(term.attributes, term.location, std::move(ast));
  }

  auto operator()(Ast::Parens const &parens) noexcept -> Ast::Ptr {
    auto ast = (*this)(parens.ast);
    return env->getParensAst(parens.attributes, parens.location,
                             std::move(ast));
  }

  auto operator()(Ast::Value const &value) noexcept -> Ast::Ptr {
    AstValueCloneVisitor visitor{env};
    return visitor(value);
  }

  auto operator()(Ast::Variable const &var) noexcept -> Ast::Ptr {
    return env->getVariableAst(var.attributes, var.location, var.name);
  }
};

[[nodiscard]] inline auto clone(Ast::Ptr const &ast, Environment *env) noexcept
    -> Ast::Ptr {
  AstCloneVisitor visitor{env};
  return visitor(ast);
}

} // namespace mint
