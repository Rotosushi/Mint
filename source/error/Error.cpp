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
#include "error/Error.hpp"

#include "utility/FatalError.hpp"

namespace mint {
auto Error::KindToView(Error::Kind kind) noexcept -> std::string_view {
  switch (kind) {
  case Error::EndOfInput:
    return "End of input";

  case Error::FileNotFound:
    return "File not found";
  case Error::ImportFailed:
    return "Import failed";

  case Error::UnknownToken:
    return "Unknown Token";
  case Error::UnknownBinop:
    return "Unknown Binop";

  case Error::ExpectedABasicTerm:
    return "Expected a basic term [nil, true, false, [0-9]+, ...]";
  case Error::ExpectedADeclaration:
    return "Expected a declaration [module, let]";
  case Error::ExpectedAnEquals:
    return "Expected a '='";
  case Error::ExpectedASemicolon:
    return "Expected a ';'";
  case Error::ExpectedAnIdentifier:
    return "Expected an identifier";
  case Error::ExpectedAClosingParen:
    return "Expected a ')'";
  case Error::ExpectedABeginBrace:
    return "Expected a '{'";
  case Error::ExpectedAEndBrace:
    return "Expected a '}'";
  case Error::ExpectedAString:
    return "Expected a string [\"...\"]";

  case Error::NameUnboundInScope:
    return "Name not bound in scope";
  case Error::NameAlreadyBoundInScope:
    return "Name already bound in scope";
  case Error::NameIsPrivateInScope:
    return "Name is private and unaccessable from this scope";

  case Error::UnopTypeMismatch:
    return "Unop argument type mismatch";
  case Error::BinopTypeMismatch:
    return "Binop argument types mismatch";

  default:
    fatalError("Unknown Error::Kind");
  }
}
} // namespace mint
