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
#include "ast/value/Lambda.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Lambda::Lambda(Attributes attributes, Location location, Arguments arguments,
               type::Ptr result_type, ast::Ptr body) noexcept
    : Value(Ast::Kind::Lambda, attributes, location),
      m_arguments(std::move(arguments)), m_result_type(result_type),
      m_body(std::move(body)) {}

auto Lambda::create(Attributes attributes, Location location,
                    Arguments arguments, type::Ptr result_type,
                    ast::Ptr body) noexcept -> ast::Ptr {
  return static_cast<ast::Ptr>(
      std::make_unique<Lambda>(attributes, location, std::move(arguments),
                               result_type, std::move(body)));
}

auto Lambda::classof(Ast const *ast) noexcept -> bool {
  return Ast::Kind::Lambda == ast->kind();
}

[[nodiscard]] auto Lambda::arguments() const noexcept -> Arguments const & {
  return m_arguments;
}
[[nodiscard]] auto Lambda::result_type() const noexcept -> type::Ptr {
  return m_result_type;
}
[[nodiscard]] auto Lambda::body() const noexcept -> ast::Ptr const & {
  return m_body;
}

auto Lambda::getLambdaName(IdentifierSet *set) noexcept -> Identifier {
  static std::size_t count = 0U;
  std::string name{"l"};
  name += std::to_string(count++);
  return set->emplace(std::move(name));
}

Ptr Lambda::clone_impl() const noexcept {
  return create(attributes(), location(), m_arguments, m_result_type,
                m_body->clone());
}

void Lambda::print(std::ostream &out) const noexcept {
  out << "\\";

  auto size = m_arguments.size();
  auto index = 0U;
  for (auto argument : m_arguments) {
    out << argument.name << ": " << argument.type;

    if (index++ < (size - 1))
      out << ", ";
  }

  if (m_result_type != nullptr)
    out << " -> " << m_result_type;

  out << " => " << m_body;
}

// #NOTE: the type of a lambda is a lambda type, wrapping
// a function type composed of the type of each of the
// arguments and result type.
// #TODO: allow for an optional return type, iff it can be
// deduced from the type of the body.
Result<type::Ptr> Lambda::typecheck(Environment &env) const noexcept {
  env.pushScope();
  type::Lambda::Arguments argument_types;

  for (auto &argument : m_arguments) {
    argument_types.emplace_back(argument.type);
    env.partialBindName(argument.name, argument.attributes, argument.type);
  }

  auto result = m_body->typecheck(env);
  if (!result) {
    env.popScope();
    return result;
  }
  auto result_type = result.value();
  auto lambda_type = env.getLambdaType(result_type, std::move(argument_types));

  env.popScope();
  return cachedType(lambda_type);
}

Result<ast::Ptr> Lambda::evaluate([[maybe_unused]] Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  return shared_from_this();
}

// #NOTE: we need to emit a llvm function object:
//   -) construct the llvm::Function
//   -) add it's basic blocks
//   -) set up the env to emit instructions
//      there.
//   -) construct space for arguments in the body
//      (not technically necessary yet due to there
//       being no assignment yet.)
//   -) create bindings for all of the arguments.
//   -) emit the body into the basic blocks.
//   -) put everything back where it was
//   -) return the function pointer as the result.
Result<llvm::Value *> Lambda::codegen(Environment &env) noexcept {
  // #NOTE: enforce that typecheck was called before
  MINT_ASSERT(cachedTypeOrAssert());
  env.pushScope();

  auto llvm_function_type =
      llvm::cast<llvm::FunctionType>(cachedTypeOrAssert()->toLLVM(env));
  auto llvm_lambda_name = getLambdaName(m_arguments.front().name.getSet());
  auto function_callee =
      env.getOrInsertFunction(llvm_lambda_name, llvm_function_type);
  auto llvm_function = llvm::cast<llvm::Function>(function_callee.getCallee());

  auto entry = env.createBasicBlock(llvm_function);
  auto temp_ip = env.exchangeInsertionPoint({entry, entry->begin()});

  auto cleanup = [&]() {
    env.exchangeInsertionPoint(temp_ip);
    env.popScope();
  };

  auto llvm_arg_cursor = llvm_function->arg_begin();
  for (auto &argument : m_arguments) {
    auto &llvm_arg = *llvm_arg_cursor;
    auto result =
        env.partialBindName(argument.name, argument.attributes, argument.type);
    if (!result) {
      cleanup();
      return result.error();
    }

    auto binding = result.value();
    binding.setRuntimeValue(&llvm_arg);
    ++llvm_arg_cursor;
  }

  auto result = m_body->codegen(env);
  if (!result) {
    cleanup();
    return result;
  }

  env.createLLVMReturn(result.value());

  cleanup();
  return llvm_function;
}
} // namespace ast
} // namespace mint