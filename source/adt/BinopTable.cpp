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
#include "adt/BinopTable.hpp"
#include "adt/Environment.hpp"

namespace mint {
/*
  #QUESTION: what is a good choice for what location and attributes
  to give the newly constructed result Ast.
  ) steal the attributes and location of one of the parameters
  ) default construct Location and Attributes.

  for now we default construct, and this is because these are
  new values being created
*/

auto eval_binop_add(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {},
                           left_integer->value() + right_integer->value());
}

auto gen_binop_add(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMAdd(left, right);
}

auto eval_binop_sub(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {},
                           left_integer->value() - right_integer->value());
}

auto gen_binop_sub(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMSub(left, right);
}

auto eval_binop_mult(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {},
                           left_integer->value() * right_integer->value());
}

auto gen_binop_mult(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMMul(left, right);
}

auto eval_binop_div(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {},
                           left_integer->value() / right_integer->value());
}

auto gen_binop_div(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMSDiv(left, right);
}

auto eval_binop_mod(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getIntegerAst({}, {},
                           left_integer->value() % right_integer->value());
}

auto gen_binop_mod(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMSRem(left, right);
}

auto eval_binop_and(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_boolean = llvm::cast<ast::Boolean>(left);
  auto *right_boolean = llvm::cast<ast::Boolean>(right);
  return env.getBooleanAst({}, {},
                           left_boolean->value() && right_boolean->value());
}

auto gen_binop_and(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMAnd(left, right);
}

auto eval_binop_or(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_boolean = llvm::cast<ast::Boolean>(left);
  auto *right_boolean = llvm::cast<ast::Boolean>(right);
  return env.getBooleanAst({}, {},
                           left_boolean->value() || right_boolean->value());
}

auto gen_binop_or(llvm::Value *left, llvm::Value *right, Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMOr(left, right);
}

auto eval_binop_integer_equality(ast::Ast *left, ast::Ast *right,
                                 Environment &env) -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() == right_integer->value());
}

auto gen_binop_integer_equality(llvm::Value *left, llvm::Value *right,
                                Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpEQ(left, right);
}

auto eval_binop_boolean_equality(ast::Ast *left, ast::Ast *right,
                                 Environment &env) -> Result<ast::Ptr> {
  auto *left_boolean = llvm::cast<ast::Boolean>(left);
  auto *right_boolean = llvm::cast<ast::Boolean>(right);
  return env.getBooleanAst({}, {},
                           left_boolean->value() == right_boolean->value());
}

auto gen_binop_boolean_equality(llvm::Value *left, llvm::Value *right,
                                Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpEQ(left, right);
}

auto eval_binop_integer_inequality(ast::Ast *left, ast::Ast *right,
                                   Environment &env) -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() != right_integer->value());
}

auto gen_binop_integer_inequality(llvm::Value *left, llvm::Value *right,
                                  Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpNE(left, right);
}

auto eval_binop_boolean_inequality(ast::Ast *left, ast::Ast *right,
                                   Environment &env) -> Result<ast::Ptr> {
  auto *left_boolean = llvm::cast<ast::Boolean>(left);
  auto *right_boolean = llvm::cast<ast::Boolean>(right);
  return env.getBooleanAst({}, {},
                           left_boolean->value() != right_boolean->value());
}

auto gen_binop_boolean_inequality(llvm::Value *left, llvm::Value *right,
                                  Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpNE(left, right);
}

auto eval_binop_less_than(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() < right_integer->value());
}

auto gen_binop_less_than(llvm::Value *left, llvm::Value *right,
                         Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpSLT(left, right);
}

auto eval_binop_less_than_or_equal(ast::Ast *left, ast::Ast *right,
                                   Environment &env) -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() <= right_integer->value());
}

auto gen_binop_less_than_or_equal(llvm::Value *left, llvm::Value *right,
                                  Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpSLE(left, right);
}

auto eval_binop_greater_than(ast::Ast *left, ast::Ast *right, Environment &env)
    -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() > right_integer->value());
}

auto gen_binop_greater_than(llvm::Value *left, llvm::Value *right,
                            Environment &env) -> Result<llvm::Value *> {
  return env.createLLVMICmpSGT(left, right);
}

auto eval_binop_greater_than_or_equal(ast::Ast *left, ast::Ast *right,
                                      Environment &env) -> Result<ast::Ptr> {
  auto *left_integer = llvm::cast<ast::Integer>(left);
  auto *right_integer = llvm::cast<ast::Integer>(right);
  return env.getBooleanAst({}, {},
                           left_integer->value() >= right_integer->value());
}

auto gen_binop_greater_than_or_equal(llvm::Value *left, llvm::Value *right,
                                     Environment &env)
    -> Result<llvm::Value *> {
  return env.createLLVMICmpSGE(left, right);
}

void InitializeBuiltinBinops(Environment *env) {
  auto integer_type = env->getIntegerType();
  auto boolean_type = env->getBooleanType();

  auto plus = env->createBinop(Token::Plus);
  plus.emplace(integer_type, integer_type, integer_type, eval_binop_add,
               gen_binop_add);

  auto minus = env->createBinop(Token::Minus);
  minus.emplace(integer_type, integer_type, integer_type, eval_binop_sub,
                gen_binop_sub);

  auto mult = env->createBinop(Token::Star);
  mult.emplace(integer_type, integer_type, integer_type, eval_binop_mult,
               gen_binop_mult);

  auto div = env->createBinop(Token::Divide);
  div.emplace(integer_type, integer_type, integer_type, eval_binop_div,
              gen_binop_div);

  auto mod = env->createBinop(Token::Modulo);
  mod.emplace(integer_type, integer_type, integer_type, eval_binop_mod,
              gen_binop_mod);

  auto boolean_and = env->createBinop(Token::And);
  boolean_and.emplace(boolean_type, boolean_type, boolean_type, eval_binop_and,
                      gen_binop_and);

  auto boolean_or = env->createBinop(Token::Or);
  boolean_or.emplace(boolean_type, boolean_type, boolean_type, eval_binop_or,
                     gen_binop_or);

  auto equality = env->createBinop(Token::EqualEqual);
  equality.emplace(integer_type, integer_type, boolean_type,
                   eval_binop_integer_equality, gen_binop_integer_equality);
  equality.emplace(boolean_type, boolean_type, boolean_type,
                   eval_binop_boolean_equality, gen_binop_boolean_equality);

  auto inequality = env->createBinop(Token::NotEqual);
  inequality.emplace(integer_type, integer_type, boolean_type,
                     eval_binop_integer_inequality,
                     gen_binop_integer_inequality);
  inequality.emplace(boolean_type, boolean_type, boolean_type,
                     eval_binop_boolean_inequality,
                     gen_binop_boolean_inequality);

  auto less = env->createBinop(Token::LessThan);
  less.emplace(integer_type, integer_type, boolean_type, eval_binop_less_than,
               gen_binop_less_than);

  auto less_or_equal = env->createBinop(Token::LessThanOrEqual);
  less_or_equal.emplace(integer_type, integer_type, boolean_type,
                        eval_binop_less_than_or_equal,
                        gen_binop_less_than_or_equal);

  auto greater = env->createBinop(Token::GreaterThan);
  greater.emplace(integer_type, integer_type, boolean_type,
                  eval_binop_greater_than, gen_binop_greater_than);

  auto greater_or_equal = env->createBinop(Token::GreaterThanOrEqual);
  greater_or_equal.emplace(integer_type, integer_type, boolean_type,
                           eval_binop_greater_than_or_equal,
                           gen_binop_greater_than_or_equal);
}

} // namespace mint
