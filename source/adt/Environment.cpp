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
#include "adt/Environment.hpp"
#include "ast/Ast.hpp"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"

namespace mint {
[[nodiscard]] auto Environment::nativeCPUFeatures() noexcept -> std::string {
  std::string features;
  llvm::StringMap<bool> map;

  if (llvm::sys::getHostCPUFeatures(map)) {
    auto cursor = map.begin();
    auto end = map.end();
    auto length = map.getNumItems();
    auto index = 0U;

    while (cursor != end) {
      if (cursor->getValue()) {
        features += "+";
      } else {
        features += "-";
      }

      features += cursor->getKeyData();

      if (index < (length - 1)) {
        features += ",";
      }

      ++cursor;
      ++index;
    }
  }

  return features;
}

[[nodiscard]] auto Environment::create(std::istream *in, std::ostream *out,
                                       std::ostream *errout,
                                       std::ostream *log) noexcept
    -> Environment {
  auto context = std::make_unique<llvm::LLVMContext>();
  auto target_triple = llvm::sys::getProcessTriple();

  std::string error_message;
  auto target =
      llvm::TargetRegistry::lookupTarget(target_triple, error_message);
  if (target == nullptr) {
    abort(error_message);
  }

  auto target_machine = target->createTargetMachine(
      target_triple, llvm::sys::getHostCPUName(), nativeCPUFeatures(),
      llvm::TargetOptions{}, llvm::Reloc::Model::PIC_,
      llvm::CodeModel::Model::Small);

  auto data_layout = target_machine->createDataLayout();
  auto ir_builder = std::make_unique<llvm::IRBuilder<>>(*context);
  auto llvm_module = std::make_unique<llvm::Module>("", *context);
  llvm_module->setDataLayout(data_layout);
  llvm_module->setTargetTriple(target_triple);

  return Environment{in,
                     out,
                     errout,
                     log,
                     std::move(context),
                     std::move(llvm_module),
                     std::move(ir_builder),
                     target_machine};
}

auto Environment::repl() noexcept -> int {
  while (true) {
    *m_output << "# ";

    auto parse_result = m_parser.parse();
    if (!parse_result) {
      auto &error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;

      printErrorWithSource(error);
      continue;
    }
    auto &ast = parse_result.value();

    auto typecheck_result = ast->typecheck(*this);
    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (!error.isUseBeforeDef()) {
        printErrorWithSource(error);
        continue;
      }

      if (auto failed = bindUseBeforeDef(error, std::move(ast))) {
        printErrorWithSource(failed.value());
      }
      continue;
    }
    auto &type = typecheck_result.value();

    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      printErrorWithSource(evaluate_result.error());
      continue;
    }
    auto &value = evaluate_result.value();

    *m_output << ast << " : " << type << " => " << value << "\n";
  }

  return EXIT_SUCCESS;
}

auto Environment::compile(fs::path filename) noexcept -> int {
  auto found = fileSearch(filename);
  if (!found) {
    Error e{Error::Kind::FileNotFound, Location{}, filename.c_str()};
    e.print(*m_error_output);
    return EXIT_FAILURE;
  }
  auto &file = found.value();
  m_parser.setIstream(&file);

  /*
    Parse, Typecheck, and Evaluate each ast within the source file

    #TODO: maybe we can handle multiple source files by "import"ing
    each subsequent file into the environment created by the first
    file given. then generating the code from there.
  */
  while (true) {
    auto parse_result = m_parser.parse();
    if (!parse_result) {
      auto &error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;

      printErrorWithSource(error);
      return EXIT_FAILURE;
    }
    auto &ast = parse_result.value();

    auto typecheck_result = ast->typecheck(*this);
    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (!error.isUseBeforeDef()) {
        printErrorWithSource(error);
        return EXIT_FAILURE;
      }

      if (auto failed = bindUseBeforeDef(error, std::move(ast))) {
        printErrorWithSource(failed.value());
        return EXIT_FAILURE;
      }
      // #NOTE:
      // if the error was use-before-def, then this ast is
      // still potentially good, so we still place it into
      // the current module.
      addAstToModule(std::move(ast));
      continue;
    }

    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      auto &error = evaluate_result.error();
      if (!error.isUseBeforeDef()) {
        printErrorWithSource(evaluate_result.error());
        return EXIT_FAILURE;
      }

      if (auto failed = bindUseBeforeDef(error, std::move(ast))) {
        printErrorWithSource(failed.value());
        return EXIT_FAILURE;
      }

      addAstToModule(std::move(ast));
      continue;
    }

    addAstToModule(std::move(ast));
  }

  /*
    codegen each term within the module,
    this populates the llvm_module with
    the llvm equivalent of all terms.
  */
  for (auto &ast : m_module) {
    auto codegen_result = ast->codegen(*this);
    if (!codegen_result) {
      printErrorWithSource(codegen_result.error());
      return EXIT_FAILURE;
    }
  }

  /*
    emit the module

    #TODO: iff there is a main entry point
    emit an object file and link it with lld
    to create an executable.
    iff there is more than one main entry point
    report an error
  */
  emitLLVMIR(filename);
  return EXIT_SUCCESS;
}

std::optional<Error> Environment::bindUseBeforeDef(Error const &error,
                                                   ast::Ptr ast) noexcept {
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.qualified_undef;
  auto ubd_def_name = names.qualified_def;

  // sanity check: the ast passed in is-a Definition.
  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  // sanity check: the name of the definition is the same
  // as the name which was returned via the error.
  MINT_ASSERT(ubd_def_name == m_local_scope->qualifyName(ubd_def_ast->name()));

  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  m_use_before_def_map.insert(ubd_name, ubd_def_name,
                              m_local_scope->qualifiedName(), std::move(ast),
                              scope);
  return std::nullopt;
}

std::optional<Error>
Environment::bindUseBeforeDef(UseBeforeDefMap::Elements &elements,
                              Error const &error, ast::Ptr ast) noexcept {
  MINT_ASSERT(error.isUseBeforeDef());
  auto ubd = error.getUseBeforeDef();
  auto &names = ubd.names;
  auto &scope = ubd.scope;
  auto ubd_name = names.qualified_undef;
  auto ubd_def_name = names.qualified_def;
  auto scope_name = scope->qualifiedName();

  // sanity check: the ast passed in is-a Definition.
  auto ubd_def_ast = llvm::cast<ast::Definition>(ast.get());
  // sanity check: the name of the definition is the same
  // as the name which was returned via the error.
  MINT_ASSERT(ubd_def_name == m_local_scope->qualifyName(ubd_def_ast->name()));

  if (auto failed = ubd_def_ast->checkUseBeforeDef(ubd)) {
    return failed;
  }

  elements.emplace_back(ubd_name, ubd_def_name, scope_name, std::move(ast),
                        scope);
  return std::nullopt;
}

std::optional<Error>
Environment::resolveTypeOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    // #NOTE: we enter the local scope of the ubd definition
    // so we know that when we construct it's binding we
    // construct it in the correct scope.
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    // #NOTE: since we are resolving the ubd here, we can clear the error
    ubd_def_ast->clearUseBeforeDef();

    auto result = ubd_def_ast->typecheck(*this);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        it.being_resolved(false);
        exchangeLocalScope(old_local_scope);
        return error;
      }

      // handle another use before def error.
      bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
      old_entries.append(it);
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    m_use_before_def_map.erase(old_entries);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error>
Environment::resolveComptimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;
  UseBeforeDefMap::Range old_entries;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->evaluate(*this);
    if (!result) {
      auto &error = result.error();
      if (!error.isUseBeforeDef()) {
        it.being_resolved(false);
        exchangeLocalScope(old_local_scope);
        return error;
      }

      // handle another use before def error.
      bindUseBeforeDef(stage, error, std::move(it.ubd_def_ast()));
      old_entries.append(it);
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  if (!old_entries.empty())
    m_use_before_def_map.erase(old_entries);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
}

std::optional<Error>
Environment::resolveRuntimeValueOfUseBeforeDef(Identifier def_name) noexcept {
  UseBeforeDefMap::Elements stage;

  auto range = m_use_before_def_map.lookup(def_name);
  for (auto it : range) {
    it.being_resolved(true);
    auto old_local_scope = exchangeLocalScope(it.scope());

    auto ubd_def_ast = llvm::cast<ast::Definition>(it.ubd_def_ast().get());
    ubd_def_ast->clearUseBeforeDef();

    // sanity check that we have called typecheck on this definition.
    MINT_ASSERT(ubd_def_ast->cachedTypeOrAssert() != nullptr);

    auto result = ubd_def_ast->codegen(*this);
    if (!result) {
      return result.error();
    }

    exchangeLocalScope(old_local_scope);
    it.being_resolved(false);
  }

  // #NOTE: codegen is the last step when processing an ast.
  // so we can safely remove these entries from the ubd map
  m_use_before_def_map.erase(range);

  if (!stage.empty())
    m_use_before_def_map.insert(std::move(stage));

  return std::nullopt;
}

auto Environment::createQualifiedNameForLLVM(Identifier name) noexcept
    -> Identifier {
  auto qualified = getQualifiedName(name);
  auto view = qualified.view();
  std::string llvm_name;

  // the llvm_name is the same as the given name,
  // where "::" is replaced with "."
  auto cursor = view.begin();
  auto end = view.end();
  while (cursor != end) {
    auto c = *cursor;
    if (c == ':') {
      llvm_name += '.';
      ++cursor; // eat "::"
      ++cursor;
    } else {
      llvm_name += c;
      ++cursor; // eat the char
    }
  }

  return m_identifier_set.emplace(std::move(llvm_name));
}

auto Environment::createLLVMGlobalVariable(std::string_view name,
                                           llvm::Type *type,
                                           llvm::Constant *init) noexcept
    -> llvm::GlobalVariable * {
  auto variable = llvm::cast<llvm::GlobalVariable>(
      llvm_module->getOrInsertGlobal(name, type));

  if (init != nullptr)
    variable->setInitializer(init);

  return variable;
}

} // namespace mint
