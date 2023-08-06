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
#include <vector>

namespace mint {
namespace ir {
class Instruction;

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
// given vector it is currently residing in, thus there
// is no reason why a single instruction should be copied
// or moved. only whole vectors can be validly copied or
// moved. however, to allow for an array to be copied or
// moved, it's elements must be copyable or movable.
// so we simply have to accept an unenforcable invariant
// to the Mir class.

class Mir {
public:
  using Ir = std::vector<Instruction>;
  using iterator = Ir::iterator;
  using const_iterator = Ir::const_iterator;
  using reference = Ir::reference;

private:
  Ir m_ir;

public:
  Mir() noexcept;
  Mir(Ir ir) noexcept;
  Mir(Mir const &other) noexcept;
  Mir(Mir &&other) noexcept;
  auto operator=(Mir const &other) noexcept -> Mir &;
  auto operator=(Mir &&other) noexcept -> Mir &;

  [[nodiscard]] auto empty() const noexcept -> bool;
  [[nodiscard]] auto size() const noexcept -> std::size_t;

  [[nodiscard]] auto ir() noexcept -> Ir &;
  [[nodiscard]] auto ir() const noexcept -> Ir const &;

  [[nodiscard]] auto begin() noexcept -> iterator;
  [[nodiscard]] auto end() noexcept -> iterator;
  [[nodiscard]] auto begin() const noexcept -> const_iterator;
  [[nodiscard]] auto end() const noexcept -> const_iterator;

  template <class... Args> [[nodiscard]] reference emplace_back(Args &&...args);
};
} // namespace ir
} // namespace mint
