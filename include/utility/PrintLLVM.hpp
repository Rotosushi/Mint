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

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_os_ostream.h"

namespace mint {
inline void print(std::ostream &out, llvm::Error const &error) noexcept {
  llvm::raw_os_ostream stream{out};
  stream << error;
}

inline std::ostream &operator<<(std::ostream &out,
                                llvm::Error const &error) noexcept {
  print(out, error);
  return out;
}

inline void print(std::ostream &out, llvm::Type const *type) noexcept {
  llvm::raw_os_ostream stream{out};
  type->print(stream);
}

inline std::ostream &operator<<(std::ostream &out,
                                llvm::Type const *type) noexcept {
  print(out, type);
  return out;
}

inline void print(std::ostream &out, llvm::Value const *value) noexcept {
  llvm::raw_os_ostream stream{out};
  value->print(stream);
}

inline std::ostream &operator<<(std::ostream &out,
                                llvm::Value const *value) noexcept {
  print(out, value);
  return out;
}
} // namespace mint
