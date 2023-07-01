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

[[nodiscard]] auto Environment::create(Allocator &resource, std::istream *in,
                                       std::ostream *out,
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

  return Environment{resource,
                     in,
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
        auto failed = handleUseBeforeDef(error, ast);
        if (failed) {
          printErrorWithSource(failed.value());
        }
        continue;
      } else {
        printErrorWithSource(error);
        continue;
      }
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

std::optional<Error> Environment::handleUseBeforeDef(const Error &error,
                                                     ast::Ptr &ast) noexcept {
  auto &use_before_def = error.getUseBeforeDef();
  auto &undef_name = use_before_def.undef;
  auto &def_name = use_before_def.def;
  // sanity check that the Ast we are currently processing
  // is a Definition itself. as it only makes sense
  // to bind a Definition in the use-before-def-map
  auto *def_ast = cast<ast::Definition>(ast.get());
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
      message << "Definition of [" << def_name << "] relies upon itself ["
              << undef_name << "]";
      Error e{Error::Kind::TypeCannotBeResolved, ast->location(),
              message.view()};
      return e;
    }
  }
  // sanity check that the name of the Definition is the same
  // as the name returned by the use-before-def Error.
  MINT_ASSERT(def_name == getQualifiedName(def_ast->name()));
  // 1) create an entry within the use-before-def-map for this
  // use-before-def Error
  bindUseBeforeDef(undef_name, def_name, ast);
  // 2) attempt to resolve any existing use-before-def entries
  // iff this error has a type annotation, then we can use that
  // to potentially type an existing entry in the use-before-def-map
  auto annotation = def_ast->annotation();
  if (annotation.has_value()) {
    auto type = annotation.value();
    local_scope->partialBind(def_name, type);
    // try to type previous UBDs with the partialBinding.
    auto range = lookupUseBeforeDef(def_name);
    auto cursor = range.begin();
    auto end = range.end();
    while (cursor != end) {
      auto typecheck_result = cursor.ast()->typecheck(*this);
      if (!typecheck_result) {
        auto &e = typecheck_result.error();
        if (e.isUseBeforeDef()) {
          auto &ubd = e.getUseBeforeDef();
          // reinsert the ast under the new name
          use_before_def_map.reinsert(ubd.undef, cursor);
        } else {
          return e;
        }
      }

      auto type = typecheck_result.value();

      // the definition typed after we created the partial binding,
      // thus we can create the definition.
      auto evaluate_result = cursor.ast()->evaluate(*this);
      if (!evaluate_result) {
        return evaluate_result.error();
      }

      // move to the next UseBeforeDef that is dependant
      // on the partial binding in the multimap.
      ++cursor;

      // #NOTE: we do not clean up the partial binding here.
      // as it leaves open the possiblity of a definition
      // appearing after this point being defined relative
      // to this name.
    }
  }
  // #NOTE: in the case that the definition we just
  // bound in the use-before-def-map has no type annotation,
  // there is no way to create a partial binding for it.
  // thus, the undef name it is defined upon must be defined
  // at some later point for this partial binding to be resolved.
}

} // namespace mint
