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
#pragma once
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace cl = llvm::cl;

//  https://llvm.org/docs/CommandLine.html#quick-start-guide
namespace mint {
// #TODO: handle mutltiple input files.
inline cl::opt<std::string> input_filename(cl::Positional,
                                           cl::desc("<input file>"));

inline cl::list<std::string>
    include_paths("I", cl::desc("add an include path to the search space"),
                  cl::value_desc("path"));

void printVersion(llvm::raw_ostream &out) noexcept;
} // namespace mint
