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
#include "ast/visitors/HasSideEffect.hpp"

namespace mint::ast {
struct HasSideEffect {
  ast::Ptr &ptr;

  HasSideEffect(ast::Ptr &ptr) noexcept : ptr(ptr) {}

  bool operator()() noexcept { return std::visit(*this, ptr->variant); }

  bool operator()([[maybe_unused]] std::monostate &nil) noexcept {
    return false;
  }
  bool operator()([[maybe_unused]] bool &b) noexcept { return false; }
  bool operator()([[maybe_unused]] int &i) noexcept { return false; }
  bool operator()([[maybe_unused]] Identifier &i) noexcept { return false; }
  bool operator()([[maybe_unused]] Lambda &l) noexcept { return false; }
  bool operator()([[maybe_unused]] Function &f) noexcept { return true; }
  bool operator()([[maybe_unused]] Let &l) noexcept { return true; }
  bool operator()([[maybe_unused]] Binop b) noexcept { return false; }
  bool operator()([[maybe_unused]] Unop &u) noexcept { return false; }
  bool operator()([[maybe_unused]] Call &c) noexcept { return false; }
  bool operator()([[maybe_unused]] Parens &p) noexcept { return false; }
  bool operator()([[maybe_unused]] Import &i) noexcept { return true; }
  bool operator()([[maybe_unused]] Module &m) noexcept { return true; }
};

bool hasSideEffect(ast::Ptr &ptr) noexcept {
  HasSideEffect visitor(ptr);
  return visitor();
}
} // namespace mint::ast
