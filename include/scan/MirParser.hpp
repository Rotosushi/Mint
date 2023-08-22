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
#include <bitset>

#include "adt/Attributes.hpp"
#include "adt/Result.hpp"
#include "adt/SourceBufferList.hpp"
#include "ir/Mir.hpp"
#include "scan/Lexer.hpp"

namespace mint {
class Environment;

class MirParser {
public:
private:
  Environment *m_env;
  SourceBufferList m_sources;
  Lexer m_lexer;
  Token m_current_token;
  Attributes m_attributes;

public:
  MirParser(Environment &env) noexcept;

  void pushSourceFile(std::fstream &&fin) {
    m_lexer.exchangeSource(m_sources.push(std::move(fin)));
  }

  void popSourceFile() { m_lexer.exchangeSource(m_sources.pop()); }

  bool endOfInput() const noexcept {
    return m_lexer.eof() && m_lexer.endOfInput();
  }

  Result<ir::Mir> parse() {
    ir::Mir mir;

    auto result = parseTop(mir);
    if (!result) {
      return result.error();
    }

    return mir;
  }

private:
  std::string_view text() const noexcept { return m_lexer.viewToken(); }
  Location location() const noexcept { return m_lexer.currentLocation(); }
  SourceLocation source() const noexcept {
    return m_lexer.source(m_lexer.currentLocation());
  }
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
    auto at_end = m_current_token == Token::End;
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

  Error recover(Error::Kind kind) noexcept {
    return recover(kind, source(), "");
  }

  Result<ir::detail::Parameter> parseTop(ir::Mir &mir);
  Result<ir::detail::Parameter> parseModule(ir::Mir &mir);
  Result<ir::detail::Parameter> parseImport(ir::Mir &mir);
  Result<ir::detail::Parameter> parseLet(ir::Mir &mir);
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
