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
#include "ast/statement/Import.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Ptr Import::clone(Environment &env) const noexcept {
  return env.getImportAst(attributes(), location(), m_filename);
}

Result<type::Ptr> Import::typecheck(Environment &env) const noexcept {
  if (env.alreadyImported(m_filename)) {
    setCachedType(env.getNilType());
    return env.getNilType();
  }

  auto exists = env.fileExists(m_filename);
  if (!exists)
    return {Error::Kind::FileNotFound, location(), m_filename};

  setCachedType(env.getNilType());
  return env.getNilType();
}

Result<ast::Ptr> Import::evaluate(Environment &env) noexcept {
  if (env.alreadyImported(m_filename)) {
    return env.getNilAst({}, {});
  }

  auto found = env.fileSearch(m_filename);
  if (!found)
    return {Error::Kind::FileNotFound, location(), m_filename};
  auto &file = found.value();
  Parser parser{&env, &file};

  while (!parser.endOfInput()) {
    auto parse_result = parser.parse();
    if (!parse_result) {
      auto &error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;
      env.printErrorWithSource(error, parser);
      return {Error::Kind::ImportFailed, location(), m_filename};
    }
    auto &ast = parse_result.value();

    auto typecheck_result = ast->typecheck(env);
    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (error.isUseBeforeDef()) {
        if (auto failed = env.bindUseBeforeDef(error, std::move(ast))) {
          env.printErrorWithSource(failed.value());
          return {Error::Kind::ImportFailed, location(), m_filename};
        }
      } else {
        env.printErrorWithSource(error, parser);
        return {Error::Kind::ImportFailed, location(), m_filename};
      }

      env.addAstToModule(std::move(ast));
      continue;
    }

    auto evaluate_result = ast->evaluate(env);
    if (!evaluate_result) {
      auto &error = evaluate_result.error();
      env.printErrorWithSource(error, parser);
      return {Error::Kind::ImportFailed, location(), m_filename};
    }

    env.addAstToModule(std::move(ast));
  }

  // #NOTE: since we just imported this file into
  // the environment, we already have it's definitions.
  // so in order to prevent redefining anything we
  // add this file to the set of imported files.
  // thus, we can later check to see if we need to
  // perform the import of this file.
  // #NOTE: this only works in a single threaded context.
  env.addImport(m_filename);
  return env.getNilAst({}, {});
}

/*
  #NOTE: the import statement is a no-op at runtime.
  as it is fully resolved at compile time.
*/
Result<llvm::Value *> Import::codegen(Environment &env) noexcept {
  return env.getLLVMNil();
}
} // namespace ast
} // namespace mint