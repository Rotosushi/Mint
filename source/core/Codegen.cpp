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
#include "core/Codegen.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"
#include "runtime/Allocate.hpp"
#include "runtime/Load.hpp"
#include "utility/VerifyLLVM.hpp"

namespace mint {
struct CodegenScalar {
  Environment *env;

  CodegenScalar(Environment &env) noexcept : env(&env) {}

  Result<llvm::Value *> operator()(ir::Scalar &scalar) noexcept {
    return std::visit(*this, scalar.variant());
  }

  Result<llvm::Value *>
  operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return env->getLLVMNil();
  }

  Result<llvm::Value *> operator()(bool &boolean) noexcept {
    return env->getLLVMBoolean(boolean);
  }

  Result<llvm::Value *> operator()(int &integer) noexcept {
    return env->getLLVMInteger(integer);
  }
};

static Result<llvm::Value *> codegen(ir::Scalar &scalar,
                                     Environment &env) noexcept {
  CodegenScalar visitor(env);
  return visitor(scalar);
}

// struct CodegenValue {
//   Environment *env;

//   CodegenValue(Environment &env) noexcept : env(&env) {}

//   Result<llvm::Value *> operator()(ir::Value &value) noexcept {
//     return std::visit(*this, value.variant());
//   }

//   Result<llvm::Value *> operator()(ir::Scalar &scalar) noexcept {
//     return codegen(scalar, *env);
//   }

// Result<llvm::Value *> operator()(ir::Lambda &lambda) noexcept {
//   env->pushScope();

//   auto type = lambda.cachedType();
//   MINT_ASSERT(type != nullptr);
//   MINT_ASSERT(type->holds<type::Lambda>());
//   auto &lambda_type = type->get<type::Lambda>();

//   auto lambda_name = env->getLambdaName();
//   auto llvm_function_type = llvm::cast<llvm::FunctionType>(
//       type::toLLVM(lambda_type.function_type, *env));
//   auto llvm_callee =
//       env->getOrInsertFunction(lambda_name, llvm_function_type);
//   auto llvm_function = llvm::cast<llvm::Function>(llvm_callee.getCallee());

//   auto entry_block = env->createBasicBlock(llvm_function);
//   auto temp_insertion_point =
//       env->exchangeInsertionPoint({entry_block, entry_block->begin()});

//   auto llvm_arguments_cursor = llvm_function->arg_begin();
//   for (auto &argument : lambda.arguments()) {
//     auto &llvm_argument = *llvm_arguments_cursor;
//     auto result = env->declareName(argument);
//     if (!result) {
//       env->exchangeInsertionPoint(temp_insertion_point);
//       env->popScope();
//       return result.error();
//     }

//     auto binding = result.value();
//     binding.setRuntimeValue(&llvm_argument);
//     ++llvm_arguments_cursor;
//   }

//   auto result = codegen(lambda.body(), *env);
//   if (!result) {
//     env->exchangeInsertionPoint(temp_insertion_point);
//     env->popScope();
//     return result;
//   }

//   env->createLLVMReturn(result.value());

//   env->exchangeInsertionPoint(temp_insertion_point);
//   env->popScope();

//   // #NOTE: verify returns true on failure.
//   // it is intended for use within an early return if statement.
//   MINT_ASSERT(!verify(*llvm_function, env->getErrorStream()));

//   return llvm_function;
// }
// };

// static Result<llvm::Value *> codegen(ir::Value &value,
//                                      Environment &env) noexcept {
//   CodegenValue visitor(env);
//   return visitor(value);
// }

struct CodegenImmediate {
  Environment *env;

  CodegenImmediate(Environment &env) noexcept : env(&env) {}

  Result<llvm::Value *> operator()(ir::detail::Immediate &immediate) noexcept {
    return std::visit(*this, immediate.variant());
  }

  Result<llvm::Value *> operator()(ir::Scalar &scalar) noexcept {
    return codegen(scalar, *env);
  }

  Result<llvm::Value *> operator()(Identifier &name) noexcept {
    auto result = env->lookupBinding(name);
    MINT_ASSERT(result.success());
    auto binding = result.value();

    if (!binding.hasRuntimeValue()) {
      return Recovered{};
    }
    auto type = type::toLLVM(binding.type(), *env);
    auto value = binding.runtimeValueOrAssert();

    if (!env->hasInsertionPoint()) {
      auto global = llvm::cast<llvm::GlobalVariable>(value);
      return global->getInitializer();
    }

    // #NOTE: runtime variables are stored in memory.
    // so they must be loaded to be used in expressions.
    return createLLVMLoad(*env, type, value);
  }
};

static Result<llvm::Value *> codegen(ir::detail::Immediate &immediate,
                                     Environment &env) noexcept {
  CodegenImmediate visitor(env);
  return visitor(immediate);
}

static Result<llvm::Value *> codegen(ir::detail::Index index, ir::Mir &mir,
                                     Environment &env) noexcept;

static Result<llvm::Value *> codegen(ir::detail::Parameter &parameter,
                                     ir::Mir &mir, Environment &env) noexcept {
  return codegen(parameter.index(), mir, env);
}

struct CodegenInstruction {
  ir::Mir *mir;
  ir::detail::Index index;
  Environment *env;

  CodegenInstruction(ir::Mir &mir, ir::detail::Index index,
                     Environment &env) noexcept
      : mir(&mir), index(index), env(&env) {}

  Result<llvm::Value *> operator()() noexcept {
    return std::visit(*this, (*mir)[index].variant());
  }

  Result<llvm::Value *> operator()(ir::detail::Immediate &immediate) noexcept {
    return codegen(immediate, *env);
  }

  Result<llvm::Value *> operator()(ir::Parens &parens) noexcept {
    return codegen(parens.parameter(), *mir, *env);
  }

  Result<llvm::Value *> operator()(ir::Let &let) noexcept {
    auto found = env->lookupLocalBinding(let.name());
    MINT_ASSERT(found);
    auto binding = found.value();

    if (binding.hasRuntimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, let.sourceLocation(),
                   let.name()};
    }

    llvm::Value *value = nullptr;
    if (binding.hasComptimeValue()) {
      auto result = codegen(binding.comptimeValueOrAssert(), *env);
      if (!result) {
        return result;
      }
      value = result.value();
    } else {
      auto result = codegen(let.parameter(), *mir, *env);
      if (!result) {
        return result;
      }
      value = result.value();
    }
    MINT_ASSERT(value != nullptr);

    auto type = let.parameter().cachedType();
    MINT_ASSERT(type != nullptr);
    auto llvm_type = type::toLLVM(type, *env);
    auto qualified_name = env->qualifyName(let.name());
    auto llvm_name = qualified_name.convertForLLVM();

    auto variable = createLLVMVariable(*env, llvm_name, llvm_type, value);

    binding.setRuntimeValue(variable);

    if (auto failed = env->resolveRuntimeValueOfUseBeforeDef(qualified_name)) {
      return failed.value();
    }

    return env->getLLVMNil();
  }

  Result<llvm::Value *> operator()(ir::Binop &binop) noexcept {
    MINT_ASSERT(env->hasInsertionPoint());
    MINT_ASSERT(binop.cachedType() != nullptr);

    auto overloads = env->lookupBinop(binop.op());
    MINT_ASSERT(overloads);

    auto left_result = codegen(binop.left(), *mir, *env);
    if (!left_result) {
      return left_result;
    }
    auto left_value = left_result.value();
    auto left_type = binop.left().cachedType();
    MINT_ASSERT(left_type != nullptr);

    auto right_result = codegen(binop.right(), *mir, *env);
    if (!right_result) {
      return right_result;
    }
    auto right_value = right_result.value();
    auto right_type = binop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(left_type, right_type);
    MINT_ASSERT(instance);

    return instance->codegen(left_value, right_value, *env);
  }

  Result<llvm::Value *> operator()(ir::Unop &unop) noexcept {
    MINT_ASSERT(env->hasInsertionPoint());
    MINT_ASSERT(unop.cachedType() != nullptr);

    auto overloads = env->lookupUnop(unop.op());
    MINT_ASSERT(overloads);

    auto result = codegen(unop.right(), *mir, *env);
    if (!result) {
      return result;
    }
    auto right = result.value();
    auto right_type = unop.right().cachedType();
    MINT_ASSERT(right_type != nullptr);

    auto instance = overloads->lookup(right_type);
    MINT_ASSERT(instance);

    return instance->codegen(right, *env);
  }

  // Result<llvm::Value *> operator()(ir::Call &call) noexcept {
  //   MINT_ASSERT(call.cachedType() != nullptr);
  //   MINT_ASSERT(env->hasInsertionPoint());

  //   auto callee_result = codegen(call.callee(), *mir, *env);
  //   if (!callee_result) {
  //     return callee_result;
  //   }

  //   std::vector<llvm::Value *> actual_arguments;
  //   actual_arguments.reserve(call.arguments().size());
  //   for (auto &argument : call.arguments()) {
  //     auto result = codegen(argument, *mir, *env);
  //     if (!result) {
  //       return result;
  //     }
  //     actual_arguments.emplace_back(result.value());
  //   }

  //   // the callee was bound directly to a function,
  //   if (auto function =
  //   llvm::dyn_cast<llvm::Function>(callee_result.value());
  //       function != nullptr) {
  //     return env->createLLVMCall(function, actual_arguments);
  //   }

  //   // else the callee was bound to a function pointer
  //   auto type = call.callee().cachedType();
  //   MINT_ASSERT(type != nullptr);
  //   MINT_ASSERT(type->holds<type::Lambda>());
  //   auto &lambda_type = type->get<type::Lambda>();
  //   auto function_type = lambda_type.function_type;
  //   MINT_ASSERT(function_type->holds<type::Function>());

  //   auto llvm_function_type =
  //       llvm::cast<llvm::FunctionType>(type::toLLVM(function_type, *env));
  //   auto llvm_function_ptr = callee_result.value();
  //   return env->createLLVMCall(llvm_function_type, llvm_function_ptr,
  //                              actual_arguments);
  // }

  // Result<llvm::Value *> operator()(ir::Lambda &lambda) noexcept {
  //   env->pushScope();

  //   auto type = lambda.cachedType();
  //   MINT_ASSERT(type != nullptr);
  //   MINT_ASSERT(type->holds<type::Lambda>());
  //   auto &lambda_type = type->get<type::Lambda>();

  //   auto lambda_name = env->getLambdaName();
  //   auto llvm_function_type = llvm::cast<llvm::FunctionType>(
  //       type::toLLVM(lambda_type.function_type, *env));
  //   auto llvm_callee =
  //       env->getOrInsertFunction(lambda_name, llvm_function_type);
  //   auto llvm_function = llvm::cast<llvm::Function>(llvm_callee.getCallee());

  //   auto entry_block = env->createBasicBlock(llvm_function);
  //   auto temp_insertion_point =
  //       env->exchangeInsertionPoint({entry_block, entry_block->begin()});

  //   auto llvm_arguments_cursor = llvm_function->arg_begin();
  //   for (auto &argument : lambda.arguments()) {
  //     auto &llvm_argument = *llvm_arguments_cursor;
  //     auto result = env->declareName(argument);
  //     if (!result) {
  //       env->exchangeInsertionPoint(temp_insertion_point);
  //       env->popScope();
  //       return result.error();
  //     }

  //     auto binding = result.value();
  //     binding.setRuntimeValue(&llvm_argument);
  //     ++llvm_arguments_cursor;
  //   }

  //   auto result = codegen(lambda.body(), *env);
  //   if (!result) {
  //     env->exchangeInsertionPoint(temp_insertion_point);
  //     env->popScope();
  //     return result;
  //   }

  //   env->createLLVMReturn(result.value());

  //   env->exchangeInsertionPoint(temp_insertion_point);
  //   env->popScope();

  //   // #NOTE: verify returns true on failure
  //   MINT_ASSERT(!verify(*llvm_function, env->getErrorStream()));

  //   return llvm_function;
  // }

  Result<llvm::Value *> operator()(ir::Import &i) noexcept {
    MINT_ASSERT(i.cachedType() != nullptr);
    return env->getLLVMNil();
  }

  Result<llvm::Value *> operator()(ir::Module &m) noexcept {
    MINT_ASSERT(m.cachedType() != nullptr);
    env->pushScope(m.name());

    std::size_t index = 0U;
    auto &recovered_expressions = m.recovered_expressions();
    for (auto &expression : m.expressions()) {
      if (!recovered_expressions[index]) {
        auto result = codegen(expression, *env);
        if (result.recovered()) {
          continue;
        }

        if (!result) {
          env->unbindScope(m.name());
          env->popScope();
          return result;
        }
      }
      ++index;
    }

    env->popScope();
    return env->getLLVMNil();
  }
};

static Result<llvm::Value *> codegen(ir::detail::Index index, ir::Mir &mir,
                                     Environment &env) noexcept {
  CodegenInstruction visitor(mir, index, env);
  return visitor();
}

Result<llvm::Value *> codegen(ir::Mir &mir, Environment &env) {
  return codegen(mir.root(), mir, env);
}
} // namespace mint
