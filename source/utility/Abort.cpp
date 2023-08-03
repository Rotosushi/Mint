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
#include "utility/Abort.hpp"

namespace mint {
[[noreturn]] void abort(std::string_view message,
                        std::source_location location) noexcept(true) {
  log(std::cerr, message, location);

  MINT_ASSERT(false && "abort");

  std::abort();
}

[[noreturn]] void abort(std::error_code error_code,
                        std::source_location location) noexcept {
  abort(error_code.message(), location);
}

[[noreturn]] void abort(std::errc ec, std::source_location location) noexcept {
  abort(std::make_error_code(ec), location);
}
} // namespace mint