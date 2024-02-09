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
#include "comptime/Codegen.hpp"
#include "adt/Environment.hpp"
#include "comptime/Typecheck.hpp"
#include "runtime/Allocate.hpp"
#include "runtime/ForwardDeclare.hpp"
#include "runtime/InlineAsm.hpp"
#include "runtime/Load.hpp"
#include "runtime/sys/Exit.hpp"
#include "utility/VerifyLLVM.hpp"

namespace mint {
struct CodegenAst {
  ast::Ptr &ptr;
  Environment &env;

  CodegenAst(ast::Ptr &ptr, Environment &env) noexcept : ptr(ptr), env(env) {}

  Result<llvm::Value *> operator()() noexcept {
    return std::visit(*this, ptr->variant);
  }

  // a literal nil is translated to a literal llvm nil
  Result<llvm::Value *>
  operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return env.getLLVMNil();
  }

  // a literal boolean is translated to a literal llvm boolean
  Result<llvm::Value *> operator()(bool &b) noexcept {
    return env.getLLVMBoolean(b);
  }

  // a literal integer is translated to a literal llvm integer
  Result<llvm::Value *> operator()(int &i) noexcept {
    return env.getLLVMInteger(i);
  }

  // -) find the binding associated with the name in scope
  // -) if there is no insertion point for instructions,
  //    we can only handle 'loading' the initializer of a global
  //    variable.
  //    else we can simply emit a load instruction which loads
  //    from a global or a local variable.
  Result<llvm::Value *> operator()(Identifier &i) noexcept {
    auto found = env.lookupBinding(i);
    if (!found) {
      return Error{Error::Kind::NameUnboundInScope, ptr->sl, i.view()};
    }
    auto binding = found.value();
    MINT_ASSERT(binding.hasRuntimeValue());

    auto type = type::toLLVM(binding.type(), env);
    auto value = binding.runtimeValueOrAssert();

    if (!env.hasInsertionPoint()) {
      auto global = llvm::cast<llvm::GlobalVariable>(value);
      return global->getInitializer();
    }

    return createLLVMLoad(env, type, value);
  }

  // -) create a llvm function with the given name and type of
  //    this function.
  // -) create a new basic block for the function, set the basic
  //    block as the current instruction insertion point, and
  //    enter a local scope
  // -) create a binding for each argument in the function,
  //    setting it's runtime value to the llvm::Argument
  // -) codegen each of the exressions in the body of the
  //    function
  // -) emit a return instruction returning the value of the
  //    final expression in the body.
  Result<llvm::Value *> operator()(ast::Function &f) noexcept {
    auto function_type_result = typecheck(ptr, env);
    MINT_ASSERT(function_type_result);
    auto type = function_type_result.value();
    MINT_ASSERT(type->holds<type::Function>());

    auto found = env.lookupLocalBinding(f.name);
    MINT_ASSERT(found);
    auto binding = found.value();

    if (binding.hasRuntimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl, f.name};
    }

    auto qualified_name = env.qualifyName(f.name);
    auto llvm_name = qualified_name.convertForLLVM();
    auto llvm_function_type =
        llvm::cast<llvm::FunctionType>(type::toLLVM(type, env));
    auto llvm_callee = env.getOrInsertFunction(llvm_name, llvm_function_type);
    auto llvm_function = llvm::cast<llvm::Function>(llvm_callee.getCallee());

    auto entry_block = env.createBasicBlock(llvm_function);
    auto temp_ip =
        env.exchangeInsertionPoint({entry_block, entry_block->begin()});
    env.pushScope();

    auto llvm_args_cursor = llvm_function->arg_begin();
    for (auto &arg : f.arguments) {
      auto &llvm_arg = *llvm_args_cursor;
      auto result = env.declareName(arg);
      if (!result) {
        env.exchangeInsertionPoint(temp_ip);
        env.popScope();
        return result.error();
      }

      auto binding = result.value();
      binding.setRuntimeValue(&llvm_arg);
      ++llvm_args_cursor;
    }

    llvm::Value *result_value = env.getLLVMNil();
    for (auto &expr : f.body) {
      auto result = codegen(expr, env);
      if (!result) {
        env.exchangeInsertionPoint(temp_ip);
        env.popScope();
        return result;
      }

      result_value = result.value();
    }

    if (f.name == env.getIdentifier("main")) {
      sysExit(env, result_value);
      // #NOTE: the ret instruction is unreachable,
      // and necessary for llvm's static analysis.
      env.createLLVMReturn();
    } else {
      env.createLLVMReturn(result_value);
    }

    env.exchangeInsertionPoint(temp_ip);
    env.popScope();

    // #NOTE: verify returns true on failure
    MINT_ASSERT(!verify(*llvm_function, env.errorStream()));

    binding.setRuntimeValue(llvm_function);

    if (auto failed = env.resolveRuntimeValueOfUseBeforeDef(qualified_name)) {
      return failed.value();
    }

    return env.getLLVMNil();
  }

  // if the let expression is bound to a comptime value
  // codegen the comptime value to a runtime value
  // else codegen the bound expression down to a value.
  // create a llvm variable bound to said value.
  Result<llvm::Value *> operator()(ast::Let &l) noexcept {
    auto affix_type_result = typecheck(l.affix, env);
    MINT_ASSERT(affix_type_result);
    auto type = affix_type_result.value();

    auto binding = [&]() -> Bindings::Binding {
      // #NOTE:
      // if the let expression has already been bound
      // in scope, return that binding. Otherwise
      // (!assumption!) we are in the case where we are
      // codegening a let expr in a function body,
      // meaning it has already been typechecked,
      // and all we need to do is create the binding
      // within the temporary local scope of the
      // function.
      auto found = env.lookupLocalBinding(l.name);
      if (found) {
        return found.value();
      }

      auto bound = env.declareName(l.name, l.attributes, type);
      MINT_ASSERT(bound);
      return bound.value();
    }();

    if (binding.hasRuntimeValue()) {
      return Error{Error::Kind::NameAlreadyBoundInScope, ptr->sl,
                   l.name.view()};
    }

    auto result = [&]() {
      if (binding.hasComptimeValue()) {
        return codegen(binding.comptimeValueOrAssert(), env);
      } else {
        return codegen(l.affix, env);
      }
    }();
    if (!result) {
      return result;
    }
    auto value = result.value();

    auto llvm_type = type::toLLVM(type, env);
    auto qualified_name = env.qualifyName(l.name);
    auto llvm_name = qualified_name.convertForLLVM();

    // #NOTE: this function handles the difference between
    // global and local variables.
    auto variable = createLLVMVariable(env, llvm_name, llvm_type, value);

    binding.setRuntimeValue(variable);

    if (auto failed = env.resolveRuntimeValueOfUseBeforeDef(qualified_name)) {
      return failed.value();
    }

    return env.getLLVMNil();
  }

  // codegen the right and left hand sides down to values
  // call the codegen routine associated with the binop given said values
  Result<llvm::Value *> operator()(ast::Binop &b) noexcept {
    MINT_ASSERT(env.hasInsertionPoint());

    auto overloads = env.lookupBinop(b.op);
    MINT_ASSERT(overloads);

    auto left_type_result = typecheck(b.left, env);
    if (!left_type_result) {
      return left_type_result.error();
    }
    auto left_type = left_type_result.value();

    auto left_result = codegen(b.left, env);
    if (!left_result) {
      return left_result;
    }
    auto left_value = left_result.value();

    auto right_type_result = typecheck(b.right, env);
    if (!right_type_result) {
      return right_type_result.error();
    }
    auto right_type = right_type_result.value();

    auto right_result = codegen(b.right, env);
    if (!right_result) {
      return right_result;
    }
    auto right_value = right_result.value();

    auto instance = overloads->lookup(left_type, right_type);
    MINT_ASSERT(instance);

    return instance->codegen(left_value, right_value, env);
  }

  // codegen the right hand side of the unop expr into a value
  // call the codegen routine associated with the unop given said value.
  Result<llvm::Value *> operator()(ast::Unop &u) noexcept {
    MINT_ASSERT(env.hasInsertionPoint());

    auto overloads = env.lookupUnop(u.op);
    MINT_ASSERT(overloads);

    auto right_type_result = typecheck(u.right, env);
    if (!right_type_result) {
      return right_type_result.error();
    }
    auto right_type = right_type_result.value();

    auto right_result = codegen(u.right, env);
    if (!right_result) {
      return right_result;
    }
    auto right_value = right_result.value();

    auto instance = overloads->lookup(right_type);
    MINT_ASSERT(instance);

    return instance->codegen(right_value, env);
  }

  // -) codegen the callee down to a function or a lambda
  // -) codegen each argument down to a llvm::Value
  // -) codegen a call instruction of the function or function ptr.
  Result<llvm::Value *> operator()(ast::Call &c) noexcept {
    MINT_ASSERT(env.hasInsertionPoint());

    auto callee_result = codegen(c.callee, env);
    if (!callee_result) {
      return callee_result;
    }
    auto callee = callee_result.value();

    std::vector<llvm::Value *> actual_arguments;
    actual_arguments.reserve(c.arguments.size());
    for (auto &argument : c.arguments) {
      auto result = codegen(argument, env);
      if (!result) {
        return result;
      }

      actual_arguments.emplace_back(result.value());
    }

    if (auto function = llvm::dyn_cast<llvm::Function>(callee);
        function != nullptr) {
      return env.createLLVMCall(function, actual_arguments);
    }

    // else the callee is a function pointer
    auto callee_type_result = typecheck(c.callee, env);
    MINT_ASSERT(callee_type_result);
    auto type = callee_type_result.value();
    MINT_ASSERT(type->holds<type::Lambda>());
    auto &lambda_type = type->get<type::Lambda>();
    auto function_type = lambda_type.function_type;
    MINT_ASSERT(function_type->holds<type::Function>());

    auto llvm_function_type =
        llvm::cast<llvm::FunctionType>(type::toLLVM(function_type, env));
    return env.createLLVMCall({llvm_function_type, callee}, actual_arguments);
  }

  // codegen whatever is inside the parens
  Result<llvm::Value *> operator()(ast::Parens &p) noexcept {
    return codegen(p.expression, env);
  }

  // forward declare all imported statements in the current TU
  Result<llvm::Value *> operator()(ast::Import &i) noexcept {
    fs::path path = i.file;
    auto *itu = env.findImport(path);
    MINT_ASSERT(itu != nullptr);

    auto &context = itu->context();
    if (context.generated()) {
      return env.getLLVMNil();
    }

    for (auto &expression : itu->expressions()) {
      forwardDeclare(expression, env);
    }

    context.generated(true);
    return env.getLLVMNil();
  }

  // codegen all expressions in the module
  Result<llvm::Value *> operator()(ast::Module &m) noexcept {
    env.pushScope(m.name);

    for (auto &expression : m.expressions) {
      auto result = codegen(expression, env);

      if (result.recovered()) {
        continue;
      } else if (!result) {
        env.unbindScope(m.name);
        env.popScope();
        return result;
      }
    }

    env.popScope();
    return env.getLLVMNil();
  }
};

Result<llvm::Value *> codegen(ast::Ptr &ptr, Environment &env) noexcept {
  CodegenAst visitor(ptr, env);
  return visitor();
}

int codegen(Environment &env) noexcept {
  for (auto &expression : env.localExpressions()) {
    auto result = codegen(expression, env);
    if (!result) {
      env.errorStream() << result.error() << "\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
} // namespace mint
