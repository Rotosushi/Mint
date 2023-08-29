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
#include "boost/dynamic_bitset.hpp"

#include "adt/Identifier.hpp"
#include "ir/detail/IrBase.hpp"

namespace mint {
namespace ir {
class Mir;

class Module : public detail::IrBase {
public:
  using Expressions = boost::container::vector<Mir>;
  using Bitset = boost::dynamic_bitset<>;

private:
  Identifier m_name;
  Expressions m_expressions;
  Bitset m_recovered_expressions;

public:
  Module(SourceLocation *sl, Identifier name, Expressions expressions) noexcept;
  Module(Module const &other) noexcept;
  Module(Module &&other) noexcept;
  auto operator=(Module const &other) noexcept -> Module &;
  auto operator=(Module &&other) noexcept -> Module &;
  ~Module() noexcept;

  [[nodiscard]] auto name() const noexcept -> Identifier;
  [[nodiscard]] auto expressions() noexcept -> Expressions &;
  [[nodiscard]] auto recovered_expressions() noexcept -> Bitset &;
};
} // namespace ir
} // namespace mint
