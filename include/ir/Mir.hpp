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
#include "boost/container/vector.hpp"
#include <utility>

#include "adt/Identifier.hpp"
#include "ir/detail/Index.hpp"
#include "ir/expression/Binop.hpp"
#include "ir/expression/Unop.hpp"
#include "ir/value/Lambda.hpp"

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
  // #TODO: replace boost::container::vector
  // with a handrolled vector like type which
  // can handle forward declared types.
  // (#NOTE: maybe based around std::unique_ptr<T[]>)
  using Ir = boost::container::vector<Instruction>;
  using iterator = Ir::iterator;
  using const_iterator = Ir::const_iterator;
  using pointer = Ir::pointer;
  using const_pointer = Ir::const_pointer;
  using reference = Ir::reference;
  using const_reference = Ir::const_reference;

private:
  detail::Index m_index;
  Ir m_ir;

  template <class T, class... Args>
  std::pair<detail::Index, Mir::pointer> emplace_back(Args &&...args);

public:
  Mir() noexcept;
  Mir(Ir ir) noexcept;
  Mir(Mir const &other) noexcept;
  Mir(Mir &&other) noexcept;
  ~Mir() noexcept;
  auto operator=(Mir const &other) noexcept -> Mir &;
  auto operator=(Mir &&other) noexcept -> Mir &;

  [[nodiscard]] auto empty() const noexcept -> bool;
  [[nodiscard]] auto size() const noexcept -> std::size_t;
  void reserve(std::size_t size) noexcept;

  [[nodiscard]] auto index() noexcept -> detail::Index &;
  [[nodiscard]] auto index() const noexcept -> detail::Index const &;
  [[nodiscard]] auto ir() noexcept -> Ir &;
  [[nodiscard]] auto ir() const noexcept -> Ir const &;

  [[nodiscard]] auto begin() noexcept -> iterator;
  [[nodiscard]] auto end() noexcept -> iterator;
  [[nodiscard]] auto begin() const noexcept -> const_iterator;
  [[nodiscard]] auto end() const noexcept -> const_iterator;

  [[nodiscard]] auto operator[](detail::Index index) noexcept -> reference;
  [[nodiscard]] auto operator[](detail::Index index) const noexcept
      -> const_reference;

  std::pair<detail::Index, pointer> emplaceLet(Identifier name);
  std::pair<detail::Index, pointer> emplaceBinop(Token op);
  std::pair<detail::Index, pointer> emplaceCall();
  std::pair<detail::Index, pointer> emplaceUnop(Token op);
  std::pair<detail::Index, pointer> emplaceImport(std::string_view file);
  std::pair<detail::Index, pointer>
  emplaceModule(Identifier name, boost::container::vector<Mir> expressions);
  std::pair<detail::Index, pointer> emplaceLambda(FormalArguments arguments,
                                                  type::Ptr result_type);
};
} // namespace ir
} // namespace mint
