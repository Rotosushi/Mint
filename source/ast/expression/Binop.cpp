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
#include "ast/expression/Binop.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"

namespace mint {
namespace ast {
Binop::Binop(Attributes attributes, Location location, Token op, Ptr left,
             Ptr right) noexcept
    : Expression{Ast::Kind::Binop, attributes, location}, m_op{op},
      m_left{std::move(left)}, m_right{std::move(right)} {
  m_left->prevAst(this);
  m_right->prevAst(this);
}

[[nodiscard]] auto Binop::create(Attributes attributes, Location location,
                                 Token op, Ptr left, Ptr right) noexcept
    -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(std::make_unique<Binop>(
      attributes, location, op, std::move(left), std::move(right)));
}

void Binop::print(std::ostream &out) const noexcept {
  out << m_left << " " << m_op << " " << m_right;
}

auto Binop::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Binop;
}

Ptr Binop::clone_impl() const noexcept {
  return create(attributes(), location(), m_op, m_left->clone(),
                m_right->clone());
}

// ir::Binop::Op Binop::convert(Token op) noexcept {
//   switch (op) {
//   case Token::Plus:
//     return ir::Binop::Plus;
//   case Token::Minus:
//     return ir::Binop::Minus;
//   case Token::Star:
//     return ir::Binop::Star;
//   case Token::Divide:
//     return ir::Binop::Divide;
//   case Token::Modulo:
//     return ir::Binop::Modulo;
//   case Token::Not:
//     return ir::Binop::Not;
//   case Token::And:
//     return ir::Binop::And;
//   case Token::Or:
//     return ir::Binop::Or;
//   case Token::LessThan:
//     return ir::Binop::LessThan;
//   case Token::LessThanOrEqual:
//     return ir::Binop::LessThanOrEqual;
//   case Token::EqualEqual:
//     return ir::Binop::EqualEqual;
//   case Token::NotEqual:
//     return ir::Binop::NotEqual;
//   case Token::GreaterThan:
//     return ir::Binop::GreaterThan;
//   case Token::GreaterThanOrEqual:
//     return ir::Binop::GreaterThanOrEqual;
//   default:
//     abort("bad binop op token");
//   }
// }

ir::detail::Parameter
Binop::flatten_impl(ir::Mir &ir,
                    [[maybe_unused]] bool immediate) const noexcept {
  return ir.emplaceBinop(m_op, m_left->flatten_impl(ir, true),
                         m_right->flatten_impl(ir, true));
}

Result<type::Ptr> Binop::typecheck(Environment &env) const noexcept {
  auto overloads = env.lookupBinop(m_op);
  if (!overloads)
    return {Error::Kind::UnknownBinop, location(), tokenToView(m_op)};

  auto left_result = m_left->typecheck(env);
  if (!left_result)
    return left_result;
  auto left_type = left_result.value();

  auto right_result = m_right->typecheck(env);
  if (!right_result)
    return right_result;
  auto right_type = right_result.value();

  auto instance = overloads->lookup(left_type, right_type);
  if (!instance) {
    std::stringstream message;
    message << "no instance of binop [" << m_op
            << "] exists given argument types [" << left_type << ","
            << right_type << "]";
    return {Error::Kind::BinopTypeMismatch, location(), message.view()};
  }

  return cachedType(instance->result_type);
}

Result<ast::Ptr> Binop::evaluate(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  // #NOTE: this doesn't work if we store copies of
  // values in the symbol table, or pass copies of
  // values in as arguments to functions, as these
  // copies will have never been typechecked.
  MINT_ASSERT(cachedTypeOrAssert());

  auto overloads = env.lookupBinop(m_op);
  MINT_ASSERT(overloads.has_value());

  auto left_result = m_left->evaluate(env);
  if (!left_result)
    return left_result;
  auto &left_value = left_result.value();
  auto left_type = m_left->cachedTypeOrAssert();

  auto right_result = m_right->evaluate(env);
  if (!right_result)
    return right_result;
  auto &right_value = right_result.value();
  auto right_type = m_right->cachedTypeOrAssert();

  auto instance = overloads->lookup(left_type, right_type);
  MINT_ASSERT(instance.has_value());

  return {instance->evaluate(left_value.get(), right_value.get())};
}

Result<llvm::Value *> Binop::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  // #NOTE: binops must be codegen'ed within a basic block.
  MINT_ASSERT(env.hasInsertionPoint());

  auto overloads = env.lookupBinop(m_op);
  MINT_ASSERT(overloads.has_value());

  auto left_result = m_left->codegen(env);
  if (!left_result)
    return left_result;
  auto left_value = left_result.value();
  auto left_type = m_left->cachedTypeOrAssert();

  auto right_result = m_right->codegen(env);
  if (!right_result)
    return right_result;
  auto right_value = right_result.value();
  auto right_type = m_right->cachedTypeOrAssert();

  auto instance = overloads->lookup(left_type, right_type);
  MINT_ASSERT(instance.has_value());

  return {instance->codegen(left_value, right_value, env)};
}
} // namespace ast
} // namespace mint