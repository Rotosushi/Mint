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
#include <sstream>

#include "adt/Environment.hpp"
#include "ast/definition/Let.hpp"
#include "utility/LLVMToString.hpp"

namespace mint {
namespace ast {
std::optional<Error>
Let::checkUseBeforeDef(Error::UseBeforeDef &ubd) const noexcept {
  if (ubd.names.qualified_undef == ubd.names.qualified_def) {
    std::stringstream message;
    message << "Let [";
    print(message);
    message << "] relies on it's own definition.";
    return Error{Error::Kind::TypeCannotBeResolved, location(), message.view()};
  }
  return std::nullopt;
}

Ptr Let::clone(Environment &env) const noexcept {
  return env.getLetAst(attributes(), location(), annotation(), name(),
                       m_ast->clone(env));
}

Result<type::Ptr> Let::typecheck(Environment &env) const noexcept {
  if (isUseBeforeDef())
    return {getUseBeforeDef()};

  auto found = env.lookupLocalBinding(name());
  if (found) {
    return {Error::Kind::NameAlreadyBoundInScope, location(), name().view()};
  }

  /*
    #NOTE: if this let expression is use-before-def,
    then the error is returned here,
  */
  auto term_type_result = m_ast->typecheck(env);
  if (!term_type_result) {
    auto &error = term_type_result.error();
    if (error.isUseBeforeDef())
      setUseBeforeDef(error.getUseBeforeDef());
    return term_type_result;
  }
  auto &type = term_type_result.value();

  auto anno = annotation();
  if (anno.has_value()) {
    auto &annotated_type = anno.value();

    if (!annotated_type->equals(type)) {
      std::stringstream message;
      message << annotated_type << " != " << type;
      return {Error::Kind::LetTypeMismatch, location(), message.view()};
    }
  }

  // #NOTE:
  // since we could type this definition, we construct a
  // partial binding, such that definitions appearing after
  // this one and relying upon this definitions type can be
  // typechecked
  if (auto bound = env.partialBindName(name(), attributes(), type);
      bound.hasError())
    return bound.error();

  // #NOTE:
  // since we could construct a partialBinding, we check
  // if we can resolve the type of any use-before-def
  // which rely upon this one.
  if (auto failed = env.resolveTypeOfUseBeforeDef(env.getQualifiedName(name())))
    return failed.value();

  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Let::evaluate(Environment &env) noexcept {
  /*
    #NOTE: if this let definition is a use-before-def, then
    we do not want to evaluate it yet
  */
  if (isUseBeforeDef())
    return {getUseBeforeDef()};

  /*
    #RULE #NOTE: we create partial bindings during typechecking,
    and complete them during evaluation. this means we
    expect the name to be bound already in scope.
    thus we assert that the binding exists.
  */
  auto found = env.lookupLocalBinding(name());
  MINT_ASSERT(found);

  auto binding = found.value();

  if (binding.hasComptimeValue())
    return {Error::Kind::NameAlreadyBoundInScope, location(), name().view()};

  auto term_value_result = m_ast->evaluate(env);
  if (!term_value_result)
    return term_value_result;
  auto &value = term_value_result.value();

  // #NOTE #RULE
  // we bind to a clone of the value, because otherwise
  // the let expression would introduce a reference.
  // this is not the meaning of let, which introduces a
  // new variable. and as such must model the semantics of
  // a new value.
  binding.setComptimeValue(value->clone(env));

  // #NOTE: we just created the comptime value for this binding,
  // so we can evaluate any partial bindings
  // that rely on this binding
  if (auto failed =
          env.resolveComptimeValueOfUseBeforeDef(env.getQualifiedName(name())))
    return failed.value();

  return env.getNilAst({}, location());
}

/*
  create the llvm::Value of the bound expression.
  create the variable representing the value at runtime.
*/
Result<llvm::Value *> Let::codegen(Environment &env) noexcept {
  if (isUseBeforeDef())
    return {getUseBeforeDef()};

  auto found = env.lookupLocalBinding(name());
  MINT_ASSERT(found);

  auto binding = found.value();

  if (binding.hasRuntimeValue())
    return {Error::Kind::NameAlreadyBoundInScope, location(), name().view()};

  auto codegen_result = m_ast->codegen(env);
  if (!codegen_result)
    return codegen_result;
  auto value = codegen_result.value();

  auto type = m_ast->cachedTypeOrAssert();
  auto llvm_type = type->toLLVM(env);

  // create a global variable for the binding
  auto llvm_name = env.createQualifiedNameForLLVM(name());
  llvm::GlobalVariable *variable = nullptr;
  if (auto constant = llvm::dyn_cast<llvm::Constant>(value)) {
    variable =
        env.createLLVMGlobalVariable(llvm_name.view(), llvm_type, constant);
  } else {
    return {Error::Kind::GlobalInitNotConstant, m_ast->location(),
            toString(value)};
  }

  binding.setRuntimeValue(variable);

  // resolve any use-before-def relying on this name
  if (auto failed =
          env.resolveRuntimeValueOfUseBeforeDef(env.getQualifiedName(name())))
    return failed.value();

  return env.getLLVMNil();
}
} // namespace ast
} // namespace mint
