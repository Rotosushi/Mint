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
#include <forward_list>

#include "type/Type.hpp"

namespace mint {
// #TODO: we have no reason to intern types, so lets stop doing that.
class TypeInterner {
  template <class T> class Composite {
    std::forward_list<type::Type> set;

  public:
    template <class... Args> type::Ptr emplace(Args &&...args) noexcept {
      std::forward_list<type::Type> potential;
      potential.emplace_front(std::in_place_type<T>,
                              std::forward<Args>(args)...);
      type::Ptr type = &potential.front();

      for (auto &element : set)
        if (equals(type, &element))
          return &element;

      set.splice_after(set.before_begin(), potential);
      return &set.front();
    }
  };

  type::Type boolean_type;
  type::Type integer_type;
  type::Type nil_type;

  Composite<type::Function> function_types;
  Composite<type::Lambda> lamdba_types;

public:
  TypeInterner() noexcept
      : boolean_type(std::in_place_type<type::Boolean>),
        integer_type(std::in_place_type<type::Integer>),
        nil_type(std::in_place_type<type::Nil>) {}

  auto getBooleanType() noexcept -> type::Ptr { return &boolean_type; }
  auto getIntegerType() noexcept -> type::Ptr { return &integer_type; }
  auto getNilType() noexcept -> type::Ptr { return &nil_type; }

  auto getFunctionType(type::Ptr result_type,
                       type::Function::Arguments arguments) noexcept
      -> type::Ptr {
    return function_types.emplace(result_type, arguments);
  }

  auto getLambdaType(type::Ptr function_type) noexcept -> type::Ptr {
    return lamdba_types.emplace(function_type);
  }
};
} // namespace mint
