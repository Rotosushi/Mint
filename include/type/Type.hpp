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
#include <optional>
#include <ostream>
#include <variant>

#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"

#include "type/Composite.hpp"
#include "type/Scalar.hpp"

namespace mint {
class Environment;

namespace type {
struct Type {
  using Variant = std::variant<Nil, Boolean, Integer, Function, Lambda>;
  Variant variant;

  template <class... Args>
  Type(Args &&...args) noexcept : variant(std::forward<Args>(args)...) {}

  template <class T> [[nodiscard]] bool holds() const noexcept {
    return std::holds_alternative<T>(variant);
  }

  template <class T> [[nodiscard]] T &get() noexcept {
    return std::get<T>(variant);
  }
};

bool equals(Ptr left, Ptr right) noexcept;
bool callable(Ptr type) noexcept;

void print(std::ostream &out, Ptr type) noexcept;

llvm::Type *toLLVM(Ptr left, Environment &env) noexcept;

inline auto operator<<(std::ostream &out, Ptr type) -> std::ostream & {
  print(out, type);
  return out;
}
} // namespace type
} // namespace mint
