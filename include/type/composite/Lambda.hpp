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
#include "type/composite/Function.hpp"

namespace mint {
namespace type {
class Lambda : public Type {
  Function const *m_function_type;

public:
  Lambda(Function const *function_type) noexcept;

  static auto classof(type::Ptr type) noexcept -> bool;

  [[nodiscard]] auto function_type() const noexcept -> Function const *;

  [[nodiscard]] bool equals(type::Ptr type) const noexcept override;
  void print(std::ostream &out) const noexcept override;

private:
  [[nodiscard]] llvm::Type *
  toLLVMImpl(Environment &env) const noexcept override;
};
} // namespace type
} // namespace mint