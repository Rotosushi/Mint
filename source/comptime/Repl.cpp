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
#include "comptime/Repl.hpp"
#include "adt/Environment.hpp"
#include "ast/visitors/Print.hpp"
#include "comptime/Evaluate.hpp"
#include "comptime/Typecheck.hpp"

namespace mint {
[[nodiscard]] int repl(SourceFiles &unique_files, std::istream *in,
                       std::ostream *out, std::ostream *errout,
                       std::ostream *log) {
  auto env = Environment::create(unique_files, in, out, errout, log);

  while (true) {
    env.outputStream() << "# ";

    auto parse_result = env.parse();
    if (!parse_result) {
      auto error = parse_result.error();
      if (error.kind() == Error::Kind::EndOfInput)
        break;

      env.errorStream() << error;
      continue;
    }
    auto &ptr = parse_result.value();

    auto typecheck_result = typecheck(ptr, env);
    if (!typecheck_result) {
      if (typecheck_result.recovered()) {
        continue;
      }

      env.errorStream() << typecheck_result.error();
      continue;
    }
    auto type = typecheck_result.value();

    auto evaluate_result = evaluate(ptr, env);
    if (!evaluate_result) {
      if (evaluate_result.recovered()) {
        continue;
      }

      env.errorStream() << evaluate_result.error();
      continue;
    }
    auto &value = evaluate_result.value();

    env.outputStream() << ptr << ": " << type << " => " << value << "\n";
  }

  return EXIT_SUCCESS;
}
} // namespace mint
