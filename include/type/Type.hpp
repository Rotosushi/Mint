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
#include <ostream>

#include "llvm/Support/Casting.h"

namespace mint {
class Type {
public:
  using Ptr = Type const *;

  enum class Kind {
    Nil,
    Boolean,
    Integer,
  };

private:
  Kind m_kind;

public:
  Type(Kind kind) noexcept : m_kind{kind} {}
  virtual ~Type() noexcept = default;

  [[nodiscard]] auto kind() const noexcept { return m_kind; }

  [[nodiscard]] virtual bool equals(Ptr right) const noexcept = 0;
  virtual void print(std::ostream &out) const noexcept = 0;
};

inline auto operator<<(std::ostream &out, Type::Ptr type) -> std::ostream & {
  type->print(out);
  return out;
}

} // namespace mint
