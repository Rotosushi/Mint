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
#include "utility/Abort.hpp"

namespace mint {
auto Error::KindToView(Error::Kind kind) noexcept -> std::string_view {
  switch (kind) {
  case Error::Kind::EndOfInput:
    return "End of input";

  case Error::Kind::UnknownToken:
    return "Unknown Token";
  case Error::Kind::UnknownBinop:
    return "Unknown Binop";

  case Error::Kind::ExpectedABasicTerm:
    return "Expected a basic term [nil, true, false, [0-9]+, ...]";
  case Error::Kind::ExpectedADeclaration:
    return "Expected a declaration [module, let]";
  case Error::Kind::ExpectedAType:
    return "Expected a type [Nil, Boolean, Integer]";
  case Error::Kind::ExpectedAnEquals:
    return "Expected a '='";
  case Error::Kind::ExpectedASemicolon:
    return "Expected a ';'";
  case Error::Kind::ExpectedAnIdentifier:
    return "Expected an identifier";
  case Error::Kind::ExpectedAClosingParen:
    return "Expected a ')'";
  case Error::Kind::ExpectedABeginBrace:
    return "Expected a '{'";
  case Error::Kind::ExpectedAEndBrace:
    return "Expected a '}'";
  case Error::Kind::ExpectedText:
    return "Expected text [\"...\"]";

  case Error::Kind::FileNotFound:
    return "File not found";
  case Error::Kind::ImportFailed:
    return "Import failed";

  case Error::Kind::LetTypeMismatch:
    return "Bound type does not equal type annotation";

  case Error::Kind::NameUnboundInScope:
    return "Name not bound in scope";
  case Error::Kind::NameAlreadyBoundInScope:
    return "Name already bound in scope";
  case Error::Kind::NameIsPrivateInScope:
    return "Name is private and unaccessable from this scope";

  case Error::Kind::UnopTypeMismatch:
    return "Unop argument type mismatch";
  case Error::Kind::BinopTypeMismatch:
    return "Binop argument types mismatch";

  default:
    abort("Unknown Error::Kind");
  }
}
} // namespace mint
