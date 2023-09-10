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
#include "runtime/ForwardDeclare.hpp"
#include "adt/Environment.hpp"
#include "ir/Instruction.hpp"

#include "llvm/IR/DerivedTypes.h"

namespace mint {
llvm::Value *forwardDeclare(Identifier name, llvm::Type *type,
                            Environment &env) noexcept {
  if (auto function_type = llvm::dyn_cast<llvm::FunctionType>(type);
      function_type != nullptr) {
    return env.getOrInsertFunction(name, function_type).getCallee();
  }

  return env.getOrInsertGlobal(name, type);
}

struct ForwardDeclareInstruction {
  Environment *env;

  ForwardDeclareInstruction(Environment &env) noexcept : env(&env) {}

  void operator()(ir::Mir &mir) noexcept {
    std::visit(*this, mir[mir.root()].variant());
  }

  void operator()([[maybe_unused]] ir::detail::Immediate &i) noexcept {}
  void operator()([[maybe_unused]] ir::Parens &p) noexcept {}

  void operator()(ir::Let &let) noexcept {
    MINT_ASSERT(let.cachedType() != nullptr);
    auto found = env->lookupLocalBinding(let.name());
    MINT_ASSERT(found);
    auto binding = found.value();
    MINT_ASSERT(!binding.hasRuntimeValue());

    auto type = binding.type();
    MINT_ASSERT(type != nullptr);
    auto llvm_type = type::toLLVM(type, *env);
    auto llvm_name = env->qualifyName(let.name()).convertForLLVM();

    auto decl = env->getOrInsertGlobal(llvm_name, llvm_type);
    binding.setRuntimeValue(decl);

    auto failed = env->resolveForwardDeclarationValueOfUseBeforeDef(let.name());
    MINT_ASSERT(!failed);
  }

  void operator()(ir::Function &function) noexcept {}

  void operator()([[maybe_unused]] ir::Binop &binop) noexcept {}
  void operator()([[maybe_unused]] ir::Unop &unop) noexcept {}
  void operator()([[maybe_unused]] ir::Import &import) noexcept {}

  void operator()(ir::Module &m) noexcept {
    MINT_ASSERT(m.cachedType() != nullptr);
    env->pushScope(m.name());

    for (auto &expression : m.expressions()) {
      forwardDeclare(expression, *env);
    }

    env->popScope();
  }
};

void forwardDeclare(ir::Mir &mir, Environment &env) noexcept {
  ForwardDeclareInstruction visitor(env);
  return visitor(mir);
}
} // namespace mint
