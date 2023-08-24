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

#include "adt/Argument.hpp"
#include "adt/Identifier.hpp"
#include "adt/SourceLocation.hpp"
#include "ir/detail/Parameter.hpp"
#include "scan/Token.hpp"

namespace mint {
namespace ir {
class Instruction;

// #TODO: rewrite the parser to produce ir, as well as print
// #TODO: IR is kinda a misnomer if we are using it to represent
// enough of the syntax such that it can be used to recreate the
// source code which created it. it's more of a mix between an
// Intermediate Representation and an Abstract Syntax Tree.

class Mir {
public:
  // #TODO: replace boost::container::vector
  // with a handrolled vector like type which
  // can handle forward declared types.
  // (#NOTE: maybe using std::unique_ptr<T[]>)
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

  template <class T, class... Args> detail::Index emplace_back(Args &&...args);

public:
  Mir() noexcept;
  Mir(Mir const &other) noexcept;
  Mir(Mir &&other) noexcept;
  ~Mir() noexcept;
  auto operator=(Mir const &other) noexcept -> Mir &;
  auto operator=(Mir &&other) noexcept -> Mir &;

  [[nodiscard]] explicit operator bool() const noexcept;
  [[nodiscard]] auto empty() const noexcept -> bool;
  [[nodiscard]] auto size() const noexcept -> std::size_t;
  void reserve(std::size_t size) noexcept;

  [[nodiscard]] auto root() const noexcept -> detail::Index;
  [[nodiscard]] auto ir() noexcept -> Ir &;
  [[nodiscard]] auto ir() const noexcept -> Ir const &;

  [[nodiscard]] auto operator[](detail::Index index) noexcept -> reference;
  [[nodiscard]] auto operator[](detail::Index index) const noexcept
      -> const_reference;

  detail::Index emplaceAffix(SourceLocation *sl, detail::Parameter parameter);
  detail::Index emplaceParens(SourceLocation *sl, detail::Parameter parameter);
  detail::Index emplaceLet(SourceLocation *sl, Identifier name,
                           std::optional<type::Ptr> annotation,
                           detail::Parameter parameter);
  detail::Index emplaceBinop(SourceLocation *sl, Token op,
                             detail::Parameter left, detail::Parameter right);
  detail::Index emplaceUnop(SourceLocation *sl, Token op,
                            detail::Parameter right);
  detail::Index emplaceImport(SourceLocation *sl, std::string_view file);
  detail::Index emplaceModule(SourceLocation *sl, Identifier name,
                              boost::container::vector<Mir> expressions);
  detail::Index emplaceCall(SourceLocation *sl, detail::Parameter callee,
                            std::vector<detail::Parameter> arguments);
  detail::Index emplaceLambda(SourceLocation *sl, FormalArguments arguments,
                              std::optional<type::Ptr> annotation, Mir body);
};
} // namespace ir
} // namespace mint
