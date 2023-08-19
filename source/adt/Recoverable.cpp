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
#include "adt/Recoverable.hpp"

namespace mint {
// #NOTE: each visitor could just as well be called functor

struct ToErrorVisitor {
  Error operator()(Recoverable &recoverable) noexcept {
    return std::visit(*this, recoverable.data());
  }

  Error operator()([[maybe_unused]] std::monostate &nil) noexcept {
    // #NOTE: default Error's abort when printed!
    return Error::Kind::Default;
  }

  Error operator()(Recoverable::UBD &ubd) noexcept {
    return {Error::Kind::NameUnboundInScope, Location{}, ubd.undef_name};
  }
};

auto Recoverable::toError() noexcept -> Error {
  ToErrorVisitor visitor;
  return visitor(*this);
}
} // namespace mint
