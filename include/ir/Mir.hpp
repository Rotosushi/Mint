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
#include "adt/Array.hpp"
#include "ir/detail/Index.hpp"

namespace mint {
namespace ir {
class Instruction;

// #TODO: maybe write a parser for ir, as well as a print
// function, such that we can emit and read back the IR
// to a file. this might come in handy for parallel
// compilation.
// #NOTE: the existence of parenthesis, means that binops
// and unops must conservatively place parens around their
// parameters, for any parameter that is not scalar.
// just in case the programmer specified an Ast infix
// expression which broke precedence rules with one or
// more parenthesis. (theoretically speaking only if
// the parameter is itself another binop or unop,
// but that would mean looking at the Instruction referenced
// by the parameter, not just the parameter.)

class Mir {
public:
  using Ir = Array<Instruction>;
  using iterator = Ir::iterator;
  using const_iterator = Ir::const_iterator;
  using reference = Ir::reference;

private:
  detail::Index m_index;
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

  [[nodiscard]] auto index() const noexcept -> detail::Index;
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
