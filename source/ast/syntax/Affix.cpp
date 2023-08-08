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
#include "ast/syntax/Affix.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Affix::Affix(Attributes attributes, Location location, Ptr ast) noexcept
    : Syntax{Ast::Kind::Affix, attributes, location}, m_ast{std::move(ast)} {
  m_ast->prevAst(this);
}

[[nodiscard]] auto Affix::create(Attributes attributes, Location location,
                                 Ptr ast) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Affix>(attributes, location, std::move(ast)));
}

auto Affix::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Affix;
}

Ptr Affix::clone_impl() const noexcept {
  return create(attributes(), location(), m_ast->clone());
}

ir::detail::Parameter Affix::flatten_impl(ir::Mir &ir) const noexcept {
  return m_ast->flatten_impl(ir);
}

void Affix::print(std::ostream &out) const noexcept { out << m_ast << ";"; }

Result<type::Ptr> Affix::typecheck(Environment &env) const noexcept {
  auto result = m_ast->typecheck(env);
  if (!result)
    return result;

  return cachedType(result.value());
}

Result<ast::Ptr> Affix::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  return m_ast->evaluate(env);
}

Result<llvm::Value *> Affix::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  return m_ast->codegen(env);
}
} // namespace ast
} // namespace mint
