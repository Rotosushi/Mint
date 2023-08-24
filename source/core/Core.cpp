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
#include "core/Core.hpp"
#include "adt/Environment.hpp"
#include "codegen/LLVMUtility.hpp"
#include "ir/action/Print.hpp"
#include "typecheck/Typecheck.hpp"

namespace mint {
[[nodiscard]] int repl(Environment &env, bool do_print) {
  auto &out = env.getOutputStream();
  while (true) {
    if (do_print)
      out << "# ";

    auto parse_result = env.parseMir();
    if (!parse_result) {
      auto error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;

      out << error;
      continue;
    }
    auto &ir = parse_result.value();

    auto typecheck_result = typecheck(ir, env);
    if (!typecheck_result) {
      if (typecheck_result.recovered()) {
        continue;
      }

      auto error = typecheck_result.error();
      out << error;
      continue;
    }
    auto type = typecheck_result.value();

    if (do_print)
      out << ir << ": " << type << "\n";
  }

  return EXIT_SUCCESS;
}
// auto &out = env.getOutputStream();
// while (true) {
//   if (do_print)
//     out << "# ";

//   auto parse_result = env.parse();
//   if (!parse_result) {
//     auto &error = parse_result.error();
//     if (error.kind() == Error::Kind::EndOfInput)
//       break;

//     env.printErrorWithSource(error);
//     continue;
//   }
//   auto &ast = parse_result.value();

//   auto typecheck_result = ast->typecheck(env);
//   if (!typecheck_result) {
//     auto &error = typecheck_result.error();
//     if (!error.isUseBeforeDef()) {
//       env.printErrorWithSource(error);
//       continue;
//     }

//     if (auto failed = env.bindUseBeforeDef(error, ast)) {
//       env.printErrorWithSource(failed.value());
//     }
//     continue;
//   }
//   auto &type = typecheck_result.value();

//   auto evaluate_result = ast->evaluate(env);
//   if (!evaluate_result) {
//     env.printErrorWithSource(evaluate_result.error());
//     continue;
//   }
//   auto &value = evaluate_result.value();

//   if (do_print)
//     out << ast << " : " << type << " => " << value << "\n";

//   env.addAstToModule(ast);
// }

// return EXIT_SUCCESS;

//  Parse, Typecheck, and Evaluate each ast within the source file
//  then Codegen all of the terms collected and emit all of that
//  as LLVM IR.
//
//  #TODO: emit the terms into a LLVM bitcode, or object file.
//
//  #TODO: add a link step to produce a library, or executable
//
//  #TODO: I think we can handle multiple source files by "repl"ing
//  each subsequent file into the environment created by the first
//  file given. then generating the code after all files are processed.
//  this might convert to a multithreaded approach, where a thread is
//  launched per input file. but we would need some way of
//  A) bringing all of the results into a single output file and
//  B) something else I am sure I haven't thought of.

// [[nodiscard]] int compile(Environment &env) {
//   auto found = env.fileSearch(env.sourceFile());
//   if (!found) {
//     out <<
//         Error{Error::Kind::FileNotFound, Location{},
//         env.sourceFile().c_str()});
//     return EXIT_FAILURE;
//   }
//   auto &file = found.value();
//   env.pushActiveSourceFile(std::move(file));

//   auto failed = repl(env, false);
//   if (failed == EXIT_FAILURE)
//     return EXIT_FAILURE;

//   for (auto &ast : env.getModule()) {
//     auto result = ast->codegen(env);
//     if (!result) {
//       out << result.error();
//       return EXIT_FAILURE;
//     }
//   }

//   return emitLLVMIR(env.getLLVMModule(), env.sourceFile(),
//                     env.getErrorStream());
// }
} // namespace mint
