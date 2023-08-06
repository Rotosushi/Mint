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
#include <memory>

#include "ir/Instruction.hpp"

namespace mint {
namespace ir {

// #TODO: write a parser for ir, as well as a print
// function, such that we can emit and read back the IR
// to a file. this might come in handy for parallel
// compilation.
// #NOTE: the existence of parenthesis, means that binops
// and unops must conservatively place parens around their
// parameters, for any parameter that is not scalar.
// just in case the programmer specified an Ast infix
// expression which broke precedence rules with one or
// more parenthesis. (theoretically speaking only if
// the parameter is itself another binop or unop I suppose)

// #NOTE: since an instruction is representing a 'flattened'
// AST, an instruction is only valid with respect to the
// given array it is currently residing in, thus there
// is no reason why a single instruction should be copied
// or moved. only whole arrays can be validly copied or
// moved. and since we disable copying of most elements
// the array object itself can only be moved. (given that
// this is only a pointer swap, the move constructors
// are not called, (I don't think))

class Mir {
  std::unique_ptr<Instruction[]> m_array;
};
// enum Kind {
//   Let,

//   Binop,
//   Call,
//   Unop,

//   Boolean,
//   Integer,
//   Lambda,
//   Nil,
// };

} // namespace ir
} // namespace mint
