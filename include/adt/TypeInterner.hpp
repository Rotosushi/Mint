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

#include "type/All.hpp"

namespace mint {
class TypeInterner {
  template <class T> class Composite {
    std::forward_list<T> set;

  public:
    template <class... Args> T const *emplace(Args &&...args) noexcept {
      std::forward_list<T> potential;
      potential.emplace_front(std::forward<Args>(args)...);
      auto const *type = &potential.front();

      for (auto &element : set)
        if (element.equals(type))
          return &element;

      set.splice_after(set.before_begin(), potential);
      return &set.front();
    }
  };

  type::Boolean boolean_type;
  type::Integer integer_type;
  type::Nil nil_type;

  Composite<type::Function> function_types;
  Composite<type::Lambda> lamdba_types;

public:
  auto getBooleanType() const noexcept -> type::Boolean const * {
    return &boolean_type;
  }
  auto getIntegerType() const noexcept -> type::Integer const * {
    return &integer_type;
  }
  auto getNilType() const noexcept -> type::Nil const * { return &nil_type; }

  auto getFunctionType(type::Ptr result_type,
                       std::vector<type::Ptr> argument_types) noexcept
      -> type::Function const * {
    return function_types.emplace(result_type, std::move(argument_types));
  }

  auto getLambdaType(type::Function const *function_type) noexcept
      -> type::Lambda const * {
    return lamdba_types.emplace(function_type);
  }
};
} // namespace mint
