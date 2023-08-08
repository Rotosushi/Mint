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
#include "ast/expression/Unop.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"
#include "utility/Abort.hpp"

namespace mint {
namespace ast {
Unop::Unop(Attributes attributes, Location location, Token op,
           Ptr right) noexcept
    : Expression{Ast::Kind::Unop, attributes, location}, m_op{op},
      m_right{std::move(right)} {
  m_right->prevAst(this);
}

[[nodiscard]] auto Unop::create(Attributes attributes, Location location,
                                Token op, Ptr right) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Unop>(attributes, location, op, std::move(right)));
}

auto Unop::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Unop;
}

Ptr Unop::clone_impl() const noexcept {
  return create(attributes(), location(), m_op, m_right->clone());
}

ir::Unop::Op Unop::convert(Token op) noexcept {
  switch (op) {
  case Token::Minus:
    return ir::Unop::Neg;

  case Token::Not:
    return ir::Unop::Not;

  default:
    abort("bad unop token");
  }
}

ir::detail::Parameter Unop::flatten_impl(ir::Mir &ir) const noexcept {
  auto pair = ir.emplace_back<ir::Unop>(convert(m_op), ir::detail::Parameter{},
                                        ir::detail::Parameter{});
  auto &unop = pair.second->unop();
  unop.right() = m_right->flatten_impl(ir);
  return pair.first;
}

void Unop::print(std::ostream &out) const noexcept {
  out << m_op << " " << m_right;
}

Result<type::Ptr> Unop::typecheck(Environment &env) const noexcept {
  auto overloads = env.lookupUnop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownUnop, location(), tokenToView(m_op)};

  auto right_result = m_right->typecheck(env);
  if (!right_result)
    return right_result;
  auto right_type = right_result.value();

  auto instance = overloads->lookup(right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of [" << m_op << "] found for type [" << right_type
            << "]";
    return {Error::Kind::UnopTypeMismatch, m_right->location(), message.view()};
  }

  return cachedType(instance->result_type);
}

Result<ast::Ptr> Unop::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  auto overloads = env.lookupUnop(m_op);
  MINT_ASSERT(overloads.has_value());

  auto right_type = m_right->cachedTypeOrAssert();

  auto right_result = m_right->evaluate(env);
  if (!right_result)
    return right_result;
  auto &right_value = right_result.value();

  auto instance = overloads->lookup(right_type);
  MINT_ASSERT(instance.has_value());

  return instance->evaluate(right_value.get());
}

Result<llvm::Value *> Unop::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  // #NOTE: unops must be codegen'ed within a basic block.
  MINT_ASSERT(env.hasInsertionPoint());

  auto overloads = env.lookupUnop(m_op);
  MINT_ASSERT(overloads.has_value());

  auto right_type = m_right->cachedTypeOrAssert();

  auto right_result = m_right->codegen(env);
  if (!right_result)
    return right_result;
  auto right_value = right_result.value();

  auto instance = overloads->lookup(right_type);
  MINT_ASSERT(instance.has_value());

  return instance->codegen(right_value, env);
}
} // namespace ast
} // namespace mint