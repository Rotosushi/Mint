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
#include <variant>

#include "adt/SourceLocation.hpp"
#include "ast/definition/Function.hpp"
#include "ast/definition/Let.hpp"
#include "ast/expression/Binop.hpp"
#include "ast/expression/Call.hpp"
#include "ast/expression/Parens.hpp"
#include "ast/expression/Unop.hpp"
#include "ast/statement/Import.hpp"
#include "ast/statement/Module.hpp"
#include "ast/value/Lambda.hpp"

namespace mint::ast {
struct Ast {
  using Variant =
      std::variant<std::monostate, bool, int, Identifier, Lambda, Function, Let,
                   Binop, Unop, Call, Parens, Import, Module>;

  std::optional<SourceLocation *> sl;
  std::optional<type::Ptr> cached_type;
  Variant variant;

  template <class... Args>
  Ast(SourceLocation *sl, Args &&...args) noexcept
      : sl(sl), cached_type(std::nullopt),
        variant(std::forward<Args>(args)...) {}

  template <class... Args>
  Ast(Args &&...args) noexcept
      : sl(std::nullopt), cached_type(std::nullopt),
        variant(std::forward<Args>(args)...) {}

  type::Ptr setCachedType(type::Ptr type) noexcept {
    cached_type = type;
    return cached_type.value();
  }

  template <class T> [[nodiscard]] bool holds() const noexcept {
    return std::holds_alternative<T>(variant);
  }

  template <class T> [[nodiscard]] T &get() noexcept {
    MINT_ASSERT(holds<T>());
    return std::get<T>(variant);
  }
};

template <class T, class... Args>
inline Ptr create(SourceLocation *sl, Args &&...args) noexcept {
  return std::make_shared<Ast>(sl, std::in_place_type<T>,
                               std::forward<Args>(args)...);
}

template <class T, class... Args> inline Ptr create(Args &&...args) noexcept {
  return std::make_shared<Ast>(std::in_place_type<T>,
                               std::forward<Args>(args)...);
}

inline Ptr create() noexcept {
  return std::make_shared<Ast>(std::in_place_type<std::monostate>);
}

} // namespace mint::ast
