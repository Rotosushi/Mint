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
  bool run = true;

  while (run) {
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
      if (error.isUseBeforeDef()) {
        if (auto failed = bindUseBeforeDef(error, ast)) {
          printErrorWithSource(failed.value());
        }
      } else {
        printErrorWithSource(error);
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

    std::cout << ast << " : " << type << " => " << value << "\n";
  }

  return EXIT_SUCCESS;
}

std::optional<Error>
Environment::bindUseBeforeDef(const Error &error,
                              const ast::Ptr &ast) noexcept {
  auto &use_before_def = error.getUseBeforeDef();
  auto &undef_name = use_before_def.q_undef;
  auto &def_name = use_before_def.q_def;
  // sanity check that the Ast we are currently processing
  // is a Definition itself. as it only makes sense
  // to bind a Definition in the use-before-def-map
  auto def_ast = dynCast<ast::Definition>(ast.get());
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
  if (auto *let_ast = dynCast<ast::Let>(ast.get()); let_ast != nullptr) {
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
  bindUseBeforeDef(undef_name, def_name, ast);
  return std::nullopt;
}

/*
  called when a new name is defined.
  any use-before-def definitions that rely
  upon the name that was just defined are
  attempted to be resolved here.
*/
std::optional<Error> Environment::resolveUseBeforeDef(Identifier def) noexcept {
  auto range = lookupUseBeforeDef(def);
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    auto &ast = cursor.ast();

    /*
      when we resolve a use before def, we are now in a situation
      where we have already created a partial binding of the
      use before def term to it's type. via partialResolveUseBeforeDef.
      thus we have already typechecked the term being fully resolved.

    */
    // sanity check that we have already called typecheck on
    // this ast and it succeeded.
    [[maybe_unused]] auto type = ast->cachedTypeOrAssert();

    // create the full binding.
    auto evaluate_result = ast->evaluate(*this);
    if (!evaluate_result) {
      return evaluate_result.error();
    }

    ++cursor;
  }

  // remove the old use before def
  use_before_def_map.erase(range);

  return std::nullopt;
}

std::optional<Error>
Environment::partialResolveUseBeforeDef(Identifier def) noexcept {
  auto range = lookupUseBeforeDef(def);
  auto cursor = range.begin();
  auto end = range.end();
  while (cursor != end) {
    auto &ast = cursor.ast();
    // #NOTE:
    // create the partial binding if we can now
    // typecheck this definition after def was
    // partially defined.
    auto typecheck_result = ast->typecheck(*this);

    if (!typecheck_result) {
      auto &e = typecheck_result.error();
      if (!e.isUseBeforeDef())
        return e;

      // reinsert the ast under the new undef name
      auto failed = bindUseBeforeDef(e, ast);
      if (failed)
        return failed;
    }

    ++cursor;
  }

  use_before_def_map.erase(range);

  return std::nullopt;
}

} // namespace mint
