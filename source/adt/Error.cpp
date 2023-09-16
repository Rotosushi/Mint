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
#include "adt/Error.hpp"
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

  case Error::Kind::ExpectedBasic:
    return "Expected a basic term [nil, true, false, [0-9]+, ...]";
  case Error::Kind::ExpectedType:
    return "Expected a type [Nil, Boolean, Integer]";
  case Error::Kind::ExpectedEquals:
    return "Expected '='";
  case Error::Kind::ExpectedColon:
    return "Expected ':'";
  case Error::Kind::ExpectedSemicolon:
    return "Expected ';'";
  case Error::Kind::ExpectedBackSlash:
    return "Expected '\\'";
  case Error::Kind::ExpectedIdentifier:
    return "Expected an identifier";
  case Error::Kind::ExpectedBeginParen:
    return "Expected '('";
  case Error::Kind::ExpectedEndParen:
    return "Expected ')'";
  case Error::Kind::ExpectedBeginBrace:
    return "Expected '{'";
  case Error::Kind::ExpectedEndBrace:
    return "Expected '}'";
  case Error::Kind::ExpectedRightArrow:
    return "Expected '->'";
  case Error::Kind::ExpectedEqualsRightArrow:
    return "Expected '=>'";
  case Error::Kind::ExpectedText:
    return "Expected text [\"...\"]";
  case Error::Kind::ExpectedDefinition:
    return "Expected keyword 'let' or 'fn'";
  case Error::Kind::ExpectedVisibility:
    return "Expected keyword 'public' or 'private'";
  case Error::Kind::ExpectedKeywordLet:
    return "Expected keyword 'let'";
  case Error::Kind::ExpectedKeywordFn:
    return "Expected keyword 'fn'";
  case Error::Kind::ExpectedKeywordModule:
    return "Expected keyword 'module'";
  case Error::Kind::ExpectedKeywordImport:
    return "Expected keyword 'import'";

  case Error::Kind::FileNotFound:
    return "File not found";
  case Error::Kind::ImportFailed:
    return "Import failed";

  case Error::Kind::AnnotatedTypeMismatch:
    return "Actual type does not match type annotation";

  case Error::Kind::UseBeforeDef:
    return "Name use before Definition";
  case Error::Kind::TypeCannotBeResolved:
    return "Type of expression cannot be resolved";

  case Error::Kind::NameUnboundInScope:
    return "Name not bound in scope";
  case Error::Kind::NameAlreadyBoundInScope:
    return "Name already bound in scope";
  case Error::Kind::NameIsPrivateInScope:
    return "Name is private and unaccessable from this scope";

  case Error::Kind::ResultTypeMismatch:
    return "Result Type annotation doesn't match computed type";

  case Error::Kind::CannotCallType:
    return "Cannot Call object";
  case Error::Kind::ArgumentTypeMismatch:
    return "Argument Types are not equal";
  case Error::Kind::ArgumentNumberMismatch:
    return "Number of Arguments is not equal";

  case Error::Kind::UnopTypeMismatch:
    return "Unop argument type mismatch";
  case Error::Kind::BinopTypeMismatch:
    return "Binop argument types mismatch";

  case Error::Kind::GlobalInitNotConstant:
    return "Global initializer not a constant";

  default:
    abort("Bad Error::Kind");
  }
}

Error::Error(Kind kind, std::optional<SourceLocation *> sl,
             std::optional<std::string_view> message) noexcept
    : m_kind(kind), m_sl(sl), m_message(message) {}

void Error::underline(std::ostream &out, Location location,
                      std::string_view bad_source) noexcept {
  for (std::size_t i = 0; i <= bad_source.size(); ++i) {
    if ((i < location.fcolumn) || (i > location.lcolumn))
      out << " ";
    else
      out << "^";
  }
  out << "\n";
}

void Error::print(std::ostream &out) const noexcept {
  out << "Error: " << KindToView(m_kind) << "\n";

  if (m_message) {
    out << " -- " << m_message.value() << "\n";
  }

  if (m_sl) {
    auto sl = m_sl.value();
    auto &loc = sl->location();
    auto &src = sl->view();

    out << " @ [" << loc.fline << ":" << loc.fcolumn << "]\n";

    if (!src.empty()) {
      out << src << "\n";
      underline(out, loc, src);
    }
  }

  out << "\n";
}

auto Error::kind() const noexcept -> Error::Kind { return m_kind; }

} // namespace mint
