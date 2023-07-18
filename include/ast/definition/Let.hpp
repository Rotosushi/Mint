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
#include <optional>

#include "adt/Identifier.hpp"
#include "ast/definition/Definition.hpp"

namespace mint {
namespace ast {
/*
  #TODO: #MAYBE:
  do we want to allow expressions like

  if (let x = ...) { ...

  where the let expression defines a new variable
  within the scope of the if expression?
  nearly identical to:
  let x = ...;
  if (... x ...) { ...


  except limiting the scope of the variable to the
  if condition. (expressing a sublte intent implicitly)
  and reducing the line count by one.

  this kind of construct relies upon changing let expressions
  to return the defined variable as their result instead of nil
  as it is currently.
  and implementing one step of automatic conversion to bool.
  introducing implicit conversions to the language.

  we could skirt implicit conversions if we instead required the
  conditional to convert to bool explicitly:

  if (let x = ...; ... x ...) { ...

  though the idea behind this is to turn the ';' into something
  akin to the comma operator in c++. which simply evaluates it's left
  hand side and then it's right, and returns it's right as the result
  type and value.
  in that case we can allow for if expressions like:

  if (let x = ...; ... x ...
    & let y = ...; ... y ...) { ...

  which combine multiple definitions into a single if statement.
*/
class Let : public Definition {
  Ptr m_ast;

public:
  Let(Attributes attributes, Location location,
      std::optional<type::Ptr> annotation, Identifier name, Ptr ast) noexcept
      : Definition{Ast::Kind::Let, attributes, location, annotation, name},
        m_ast{std::move(ast)} {
    m_ast->setPrevAst(this);
  }
  ~Let() noexcept override = default;

  static auto classof(Ast const *ast) noexcept -> bool {
    return Ast::Kind::Let == ast->kind();
  }

  Ptr clone(Environment &env) const noexcept override;

  void print(std::ostream &out) const noexcept override {
    if (attributes().isPublic()) {
      out << "public ";
    }

    out << "let " << name();
    auto anno = annotation();
    if (anno.has_value())
      out << " : " << anno.value();
    out << " = " << m_ast;
  }

  Result<type::Ptr> typecheck(Environment &env) const noexcept override;
  Result<ast::Ptr> evaluate(Environment &env) noexcept override;
  Result<llvm::Value *> codegen(Environment &env) noexcept override;
};
} // namespace ast
} // namespace mint
