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

#include "adt/Result.hpp"
#include "ir/Mir.hpp"
#include "type/Type.hpp"

namespace mint {
class Environment;

// What does evaluate do it's work upon?
// ir::Values. Which are variants of
// all literals that are possible in the
// language. that is, ir::detail::Scalar,
// and Lambdas. however, lambdas are themselves
// only fully representable as ir::Mir's.
// so that's easy, we just use an ir::Mir as a
// standin for lambdas.
// well, okay great, what about structs, unions,
// tuples, etc?
// worry about that wehn we get there?
// they will be storable in an ir::Mir at the very least.
// but then isn't that an ambiguity? how do we know
// if the ir::Mir is a Lambda or a struct or whatever else?
// a tag works. well, iff we make ir::Lambda hold an ir::Mir to
// represent it's body, then we can use an ir::Lambda directly
// within a ir::Value, and the tag comes for free.
// it still leaves open the representation of everything else,
// but it's easy to imagine how those things can be distinguished
// once they are within the ir::Value.
// do operators work on ir::Values instead of ir::Scalars then?
// only if we decide to overload operators over more complex types
// internally.

} // namespace mint
