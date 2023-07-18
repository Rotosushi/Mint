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

#include "utility/Casting.hpp"

#include "llvm/IR/Type.h"

namespace mint {
class Environment;

namespace type {
class Type;
using Ptr = Type const *;

class Type {
public:
  enum class Kind {
    Nil,
    Boolean,
    Integer,
  };

protected:
  mutable llvm::Type *m_cached_llvm_type;

  [[nodiscard]] virtual llvm::Type *
  toLLVMImpl(Environment &env) const noexcept = 0;

private:
  Kind m_kind;

public:
  Type(Kind kind) noexcept : m_cached_llvm_type{nullptr}, m_kind{kind} {}
  virtual ~Type() noexcept = default;

  [[nodiscard]] auto kind() const noexcept { return m_kind; }

  [[nodiscard]] virtual bool equals(Ptr right) const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;

  [[nodiscard]] llvm::Type *toLLVM(Environment &env) const noexcept {
    if (m_cached_llvm_type != nullptr) {
      return m_cached_llvm_type;
    }

    return toLLVMImpl(env);
  }
};

inline auto operator<<(std::ostream &out, Ptr type) -> std::ostream & {
  type->print(out);
  return out;
}
} // namespace type
} // namespace mint
