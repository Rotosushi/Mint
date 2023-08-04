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
#include "llvm/IR/BasicBlock.h"

namespace mint {
struct InsertionPoint {
  llvm::BasicBlock *basic_block;
  llvm::BasicBlock::iterator it;

  InsertionPoint() noexcept : basic_block(nullptr) {}
  InsertionPoint(llvm::BasicBlock *bb, llvm::BasicBlock::iterator it) noexcept
      : basic_block(bb), it(it) {}

  [[nodiscard]] auto good() const noexcept -> bool {
    return basic_block != nullptr;
  }
};
} // namespace mint
