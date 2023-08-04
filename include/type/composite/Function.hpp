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

#include "type/Type.hpp"

namespace mint {
namespace type {
class Function : public Type {
public:
  using Arguments = std::vector<type::Ptr>;

private:
  type::Ptr m_result_type;
  Arguments m_arguments;

public:
  Function(type::Ptr result_type) noexcept;
  Function(type::Ptr result_type, Arguments arguments) noexcept;
  ~Function() noexcept override = default;

  static auto classof(type::Ptr type) noexcept -> bool;

  [[nodiscard]] auto result_type() const noexcept -> type::Ptr;
  [[nodiscard]] auto arguments() const noexcept -> Arguments const &;

  [[nodiscard]] bool equals(type::Ptr type) const noexcept override;
  void print(std::ostream &out) const noexcept override;

private:
  [[nodiscard]] llvm::Type *
  toLLVMImpl(Environment &env) const noexcept override;
};
} // namespace type
} // namespace mint
