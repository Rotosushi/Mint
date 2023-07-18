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
                                       std::ostream *errout) noexcept
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
                     std::move(context),
                     std::move(llvm_module),
                     std::move(ir_builder),
                     target_machine};
}

auto Environment::repl() noexcept -> int {
  while (true) {
    *out << "# ";

    auto parse_result = parser.parse();
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

      if (auto failed = bindUseBeforeDef(error, ast)) {
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

    *out << ast << " : " << type << " => " << value << "\n";
  }

  return EXIT_SUCCESS;
}

auto Environment::compile(fs::path filename) noexcept -> int {
  auto found = fileSearch(filename);
  if (!found) {
    Error e{Error::Kind::FileNotFound, Location{}, filename.c_str()};
    e.print(*errout);
    return EXIT_FAILURE;
  }
  auto &file = found.value();
  parser.setIstream(&file);

  /*
    Parse, Typecheck, and Evaluate each ast within the source file

    #TODO: maybe we can handle multiple source files by "import"ing
    each subsequent file into the environment created by the first
    file given. then generating the code from there.
  */
  while (true) {
    auto parse_result = parser.parse();
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

      if (auto failed = bindUseBeforeDef(error, ast)) {
        printErrorWithSource(failed.value());
        return EXIT_FAILURE;
      }
      // #NOTE:
      // if the error was use-before-def, then this ast is
      // still potentially good, so we still place it into
      // the current module.
      m_module.push_back(ast);
      continue;
    }

    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      auto &error = evaluate_result.error();
      if (!error.isUseBeforeDef()) {
        printErrorWithSource(evaluate_result.error());
        return EXIT_FAILURE;
      }

      if (auto failed = bindUseBeforeDef(error, ast)) {
        printErrorWithSource(failed.value());
        return EXIT_FAILURE;
      }

      m_module.push_back(ast);
      continue;
    }

    m_module.push_back(ast);
  }

  /*
    codegen each term within the module,
    this populates the llvm_module with
    the llvm equivalent of all terms.
  */
  for (auto ast : m_module) {
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

auto Environment::getQualifiedNameForLLVM(Identifier name) noexcept
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

  return identifier_set.emplace(std::move(llvm_name));
}

std::optional<Error>
Environment::bindUseBeforeDef(const Error &error,
                              const ast::Ptr &ast) noexcept {
  auto &use_before_def = error.getUseBeforeDef();
  auto &undef_name = use_before_def.undef;
  auto &def_name = use_before_def.def;
  auto &scope = use_before_def.scope;
  // sanity check that the Ast we are currently processing
  // is a Definition itself. as it only makes sense
  // to bind a Definition in the use-before-def-map
  auto def_ast = dynCast<ast::Definition>(ast);
  MINT_ASSERT(def_ast != nullptr);
  // sanity check that the name of the Definition is the same
  // as the name returned by the use-before-def Error.
  MINT_ASSERT(def_name == getQualifiedName(def_ast->name()));
  // sanity check that the use-before-def is not
  // an unresolvable trivial loop.
  // #TODO: this check needs to be made more robust moving forward.
  // let expressions obviously cannot rely upon their
  // own definition, thus they form the trivial loop.
  // however a function definition can.
  // a new type can, but only under certain circumstances.
  if (auto *let_ast = dynCast<ast::Let>(ast); let_ast != nullptr) {
    if (undef_name == def_name) {
      std::stringstream message;
      message << "Definition [" << ast << "] relies upon itself [" << undef_name
              << "]";
      Error e{Error::Kind::TypeCannotBeResolved, ast->location(),
              message.view()};
      return e;
    }
  }
  // create an entry within the use-before-def-map for this
  // use-before-def Error
  bindUseBeforeDef(undef_name, def_name, ast, scope);
  return std::nullopt;
}

std::optional<Error>
Environment::resolveTypeOfUseBeforeDef(Identifier def) noexcept {
  std::vector<
      std::tuple<Identifier, Identifier, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::Entry> old_entries;

  auto range = lookupUseBeforeDef(def);
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto definition = cursor.definition();
    auto &ast = cursor.ast();
    [[maybe_unused]] auto &def_scope = cursor.scope();
    // save the current scope, and enter the scope
    // that the definition appears in
    [[maybe_unused]] auto temp_scope = local_scope;
    local_scope = def_scope;
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();
    //  #NOTE:
    //  create the partial binding if we can now
    //  typecheck this definition after def was
    //  partially defined.
    auto typecheck_result = ast->typecheck(*this);

    if (!typecheck_result) {
      auto &error = typecheck_result.error();
      if (!error.isUseBeforeDef()) {
        local_scope = temp_scope; // restore scope
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.undef, usedef.def, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    local_scope = temp_scope; // restore scope
    ++cursor;
  }

  // remove any out of date entries in the map
  if (!old_entries.empty())
    for (auto &entry : old_entries)
      use_before_def_map.erase(entry);
  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty())
    for (auto &usedef : stage)
      use_before_def_map.insert(std::get<0>(usedef), std::get<1>(usedef),
                                std::get<2>(usedef), std::get<3>(usedef));

  return std::nullopt;
}

/*
  called when a new name is defined.
  any use-before-def definitions that rely
  upon the name that was just defined are
  attempted to be resolved here.
*/
std::optional<Error>
Environment::resolveComptimeValueOfUseBeforeDef(Identifier def) noexcept {
  std::vector<
      std::tuple<Identifier, Identifier, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::Entry> old_entries;

  auto range = lookupUseBeforeDef(def);
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto def = cursor.definition();
    auto &ast = cursor.ast();
    [[maybe_unused]] auto def_scope = cursor.scope();
    [[maybe_unused]] auto temp_scope = local_scope;
    local_scope = def_scope;
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();

    /*
      when we resolve a use before def, we are now in a situation
      where we have already created a partial binding of the
      use before def term to it's type. via resolveTypeOfUseBeforeDef.
      thus we have already typechecked the term being fully resolved.
      thus all that is needed is to fully resolve the definition.
    */
    // sanity check that we have already called typecheck on
    // this ast and it succeeded.
    [[maybe_unused]] auto type = ast->cachedTypeOrAssert();

    // create the full binding. resolving the use-before-def
    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      auto &error = evaluate_result.error();
      if (!error.isUseBeforeDef()) {
        local_scope = temp_scope;
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.undef, usedef.def, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    local_scope = temp_scope;
    ++cursor;
  }

  // remove the old use before defs
  if (!old_entries.empty())
    for (auto &entry : old_entries)
      use_before_def_map.erase(entry);

  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty()) {
    for (auto &usedef : stage)
      use_before_def_map.insert(std::get<0>(usedef), std::get<1>(usedef),
                                std::get<2>(usedef), std::get<3>(usedef));
  }

  return std::nullopt;
}

std::optional<Error>
Environment::resolveRuntimeValueOfUseBeforeDef(Identifier def) noexcept {
  std::vector<
      std::tuple<Identifier, Identifier, ast::Ptr, std::shared_ptr<Scope>>>
      stage;
  std::vector<UseBeforeDefMap::Entry> old_entries;

  auto range = lookupUseBeforeDef(def);
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    [[maybe_unused]] auto undef = cursor.undef();
    [[maybe_unused]] auto def = cursor.definition();
    auto &ast = cursor.ast();
    [[maybe_unused]] auto def_scope = cursor.scope();
    [[maybe_unused]] auto temp_scope = local_scope;
    local_scope = def_scope;
    // sanity check that the Ast we are currently processing
    // is a Definition itself. as it only makes sense
    // to bind a Definition in the use-before-def-map
    auto def_ast = cast<ast::Definition>(ast);
    // since we are attempting to resolve this UseBeforeDef
    // we clear the current UseBeforeDef error during the
    // attempt.
    def_ast->clearUseBeforeDef();

    /*
      when we resolve a use before def, we are now in a situation
      where we have already created a partial binding of the
      use before def term to it's type. via resolveTypeOfUseBeforeDef.
      thus we have already typechecked the term being fully resolved.
      thus all that is needed is to fully resolve the definition.
    */
    // sanity check that we have already called typecheck on
    // this ast and it succeeded.
    [[maybe_unused]] auto type = ast->cachedTypeOrAssert();

    // create the full binding. resolving the use-before-def
    auto codegen_result = ast->codegen(*this);
    if (!codegen_result) {
      auto &error = codegen_result.error();
      if (!error.isUseBeforeDef()) {
        local_scope = temp_scope;
        return error;
      }
      // since the ast failed to typecheck due to another
      // use-before-def we want to handle that here.
      auto &usedef = error.getUseBeforeDef();
      // sanity check that this undef name is not
      // the same as the original undef name
      MINT_ASSERT(usedef.undef != undef);

      // stage the use-before-def to be inserted into the map.
      // (so we are not inserting as we are iterating)
      stage.emplace_back(usedef.undef, usedef.def, ast, usedef.scope);
      // since this entry in the use-before-def map failed
      // with another use-before-def error, it's entry in the
      // map is out of date, thus we need to remove it.
      old_entries.emplace_back(cursor);
    }

    local_scope = temp_scope;
    ++cursor;
  }

  // remove the resolved use before defs
  // #NOTE: once the runtime value has
  // been resolved, there is nothing more
  // to resolve for a given binding.
  use_before_def_map.erase(range);

  // reinsert any definitions which failed to type
  // because of another use-before-def
  if (!stage.empty()) {
    for (auto &usedef : stage)
      use_before_def_map.insert(std::get<0>(usedef), std::get<1>(usedef),
                                std::get<2>(usedef), std::get<3>(usedef));
  }

  return std::nullopt;
}

auto Environment::createLLVMGlobalVariable(std::string_view name,
                                           llvm::Type *type,
                                           llvm::Constant *init) noexcept
    -> llvm::GlobalVariable * {
  auto variable = mint::cast<llvm::GlobalVariable>(
      llvm_module->getOrInsertGlobal(name, type));

  if (init != nullptr)
    variable->setInitializer(init);

  return variable;
}

} // namespace mint
