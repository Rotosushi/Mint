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

struct ForwardDeclareAst {
  ast::Ptr &ptr;
  Environment &env;

  ForwardDeclareAst(ast::Ptr &ptr, Environment &env) noexcept
      : ptr(ptr), env(env) {}

  void operator()() noexcept { std::visit(*this, ptr->variant); }

  void operator()([[maybe_unused]] std::monostate &nil) noexcept {}
  void operator()([[maybe_unused]] bool &b) noexcept {}
  void operator()([[maybe_unused]] int &i) noexcept {}
  void operator()([[maybe_unused]] Identifier &i) noexcept {}

  void operator()(ast::Function &f) noexcept {
    auto found = env.lookupLocalBinding(f.name);
    MINT_ASSERT(found);
    auto binding = found.value();
    MINT_ASSERT(!binding.hasRuntimeValue());

    auto type = binding.type();
    MINT_ASSERT(type != nullptr);
    auto llvm_type = type::toLLVM(type, env);
    auto llvm_name = env.qualifyName(f.name).convertForLLVM();

    auto *fwd_decl = forwardDeclare(llvm_name, llvm_type, env);
    binding.setRuntimeValue(fwd_decl);

    auto failed = env.resolveForwardDeclarationValueOfUseBeforeDef(f.name);
    MINT_ASSERT(!failed);
  }

  void operator()(ast::Let &l) noexcept {
    auto found = env.lookupLocalBinding(l.name);
    MINT_ASSERT(found);
    auto binding = found.value();
    MINT_ASSERT(!binding.hasRuntimeValue());

    auto type = binding.type();
    MINT_ASSERT(type != nullptr);
    auto llvm_type = type::toLLVM(type, env);
    auto llvm_name = env.qualifyName(l.name).convertForLLVM();

    auto fwd_decl = forwardDeclare(llvm_name, llvm_type, env);
    binding.setRuntimeValue(fwd_decl);

    auto failed = env.resolveForwardDeclarationValueOfUseBeforeDef(l.name);
    MINT_ASSERT(!failed);
  }

  void operator()([[maybe_unused]] ast::Binop &b) noexcept {}
  void operator()([[maybe_unused]] ast::Unop &u) noexcept {}
  void operator()([[maybe_unused]] ast::Call &c) noexcept {}
  void operator()([[maybe_unused]] ast::Parens &p) noexcept {}

  // #NOTE: we don't need to wal each imported definition and
  // forward declare them, because that is what codegen'ing
  // this import expression will already do. and the way we reach
  // this point, is precisely when we are codegen'ing an import
  // statement.
  void operator()([[maybe_unused]] ast::Import &i) noexcept {}

  void operator()(ast::Module &m) noexcept {
    env.pushScope(m.name);

    for (auto &expression : m.expressions) {
      forwardDeclare(expression, env);
    }

    env.popScope();
  }
};

void forwardDeclare(ast::Ptr &ptr, Environment &env) noexcept {
  MINT_ASSERT(ptr->cached_type != nullptr);
  ForwardDeclareAst visitor(ptr, env);
  visitor();
}

} // namespace mint
