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
#include <array>    // std::array<T, N>
#include <charconv> // std::to_chars, std::from_chars
#include <concepts> // std::integral
#include <limits>   // std::numeric_limits<T>::digits10
#include <string>   // std::string

#include "utility/FatalError.hpp"

namespace mint {
template <std::integral Integral>
[[nodiscard]] inline auto StringFromNumber(Integral number) noexcept
    -> std::string {
  std::array<char, std::numeric_limits<Integral>::digits10 + 1> buffer{};
  auto [ptr, errc] = std::to_chars(buffer.begin(), buffer.end(), number);
  if (errc != std::errc{}) {
    fatalError(std::make_error_code(errc).message());
  }
  return {buffer.begin(), buffer.size()};
}

template <std::floating_point Floating>
[[nodiscard]] inline auto StringFromNumber(Floating number) noexcept
    -> std::string {
  std::array<char, std::numeric_limits<Floating>::max_digits10 + 1> buffer{};
  auto [ptr, errc] = std::to_chars(buffer.begin(), buffer.end(), number);
  if (errc != std::errc{}) {
    fatalError(std::make_error_code(errc).message());
  }
  return {buffer.begin(), buffer.size()};
}

template <typename Number>
[[nodiscard]] inline auto StringToNumber(std::string_view string) -> Number {
  Number number;
  auto [ptr, errc] = std::from_chars(string.begin(), string.end(), number);
  if (errc != std::errc{}) {
    fatalError(std::make_error_code(errc).message());
  }
  return number;
}

} // namespace mint