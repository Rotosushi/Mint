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

#include "adt/Result.hpp"
#include "ir/Mir.hpp"
#include "scan/Lexer.hpp"

namespace mint {
class Environment;

class MirParser {
public:
private:
  Lexer m_lexer;
  Token m_current_token;

public:
  MirParser(SourceBuffer *source) noexcept : m_lexer(source) {}

  SourceBuffer *exchangeSource(SourceBuffer *source) noexcept {
    return m_lexer.exchangeSource(source);
  }

  bool endOfInput() const noexcept;

  Result<ir::Mir> parse();

private:
  std::string_view text() const noexcept { return m_lexer.viewToken(); }
  Location location() const noexcept { return m_lexer.currentLocation(); }
  SourceLocation source(Location const &location) const noexcept {
    return m_lexer.source(location);
  }

  void next() noexcept { m_current_token = m_lexer.lex(); }
  bool peek(Token token) noexcept {
    fill();
    return m_current_token == token;
  }
  bool expect(Token token) noexcept {
    fill();
    if (peek(token)) {
      next();
      return true;
    }
    return false;
  }

  void fill() noexcept {
    auto at_end = peek(Token::End);
    auto more_source = !m_lexer.eof();
    auto good = m_lexer.good();
    if (at_end && more_source && good) {
      m_lexer.fill();
      next();
    }
  }

  Error recover(Error::Kind kind, SourceLocation source,
                std::string_view message) noexcept {
    while (!expect(Token::Semicolon) && !peek(Token::End))
      next();

    return {kind, source, message};
  }

  Result<ir::detail::Parameter> parseTop(ir::Mir &mir);
  Result<ir::detail::Parameter> parseModule(ir::Mir &mir, bool is_public);
  Result<ir::detail::Parameter> parseImport(ir::Mir &mir, bool is_public);
  Result<ir::detail::Parameter> parseLet(ir::Mir &mir, bool is_public);
  Result<ir::detail::Parameter> parseTerm(ir::Mir &mir);

  Result<ir::detail::Parameter> parseAffix(ir::Mir &mir);
  Result<ir::detail::Parameter> parseCall(ir::Mir &mir);
  Result<ir::detail::Parameter> parseBinop(ir::Mir &mir,
                                           ir::detail::Parameter left,
                                           BinopPrecedence precedence);
  Result<ir::detail::Parameter> parseBasic(ir::Mir &mir);
  // #TODO: remember that immediates have to choose
  // to insert a new instruction or not somehow!
  Result<ir::detail::Parameter> parseNil(ir::Mir &mir);
  Result<ir::detail::Parameter> parseTrue(ir::Mir &mir);
  Result<ir::detail::Parameter> parseFalse(ir::Mir &mir);
  Result<ir::detail::Parameter> parseInteger(ir::Mir &mir);
  Result<ir::detail::Parameter> parseVariable(ir::Mir &mir);
  Result<ir::detail::Parameter> parseUnop(ir::Mir &mir);
  Result<ir::detail::Parameter> parseParens(ir::Mir &mir);
  Result<ir::detail::Parameter> parseLambda(ir::Mir &mir);

  Result<type::Ptr> parseType();
  Result<type::Ptr> parseNilType();
  Result<type::Ptr> parseBooleanType();
  Result<type::Ptr> parseIntegerType();
  Result<type::Ptr> parseFunctionType();
};
} // namespace mint
