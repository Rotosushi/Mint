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

#include "scan/Scanner.hpp"

namespace mint {
void Scanner::UpdateLocation() noexcept {
  auto length = cursor - token;
  location.fline = location.lline;
  location.fcolumn = location.lcolumn;

  for (std::ptrdiff_t i = 0; i < length; ++i) {
    if (token[i] == '\n') {
      ++location.lline;
      location.lcolumn = 0;
      location.fcolumn = 0;
    } else {
      ++location.lcolumn;
    }
  }
}

Scanner::Scanner() noexcept : location{1, 0, 1, 0} {
  end = marker = token = cursor = buffer.end();
}

Scanner::Scanner(std::string_view input) noexcept
    : location{1, 0, 1, 0}, buffer{input} {
  end = buffer.end();
  marker = token = cursor = buffer.begin();
}

auto Scanner::view() const noexcept -> std::string_view {
  return {buffer.c_str(), buffer.size()};
}

void Scanner::reset() noexcept {
  location = {1, 0, 1, 0};
  buffer.clear();
  end = cursor = marker = token = buffer.end();
}

auto Scanner::endOfInput() const noexcept -> bool { return cursor == end; }

void Scanner::append(std::string_view text) noexcept {
  if (buffer.empty()) {
    buffer.append(text);
    end = buffer.end();
    marker = token = cursor = buffer.begin();
  } else {
    auto begin = buffer.begin();
    auto cursor_offset = std::distance(begin, cursor);
    auto marker_offset = std::distance(begin, marker);
    auto token_offset = std::distance(begin, token);

    buffer.append(text);

    begin = buffer.begin();
    end = buffer.end();
    cursor = begin + cursor_offset;
    marker = begin + marker_offset;
    token = begin + token_offset;
  }
}

auto Scanner::getText() const noexcept -> std::string_view {
  return {token, cursor};
}

auto Scanner::getLocation() const noexcept -> Location { return location; }

/*!re2c
  re2c:api:style = free-form;
  re2c:yyfill:enable = 0;
  re2c:eof = 0;

  re2c:define:YYCTYPE  = "char";
  re2c:define:YYCURSOR = "cursor";
  re2c:define:YYMARKER = "marker";
  re2c:define:YYLIMIT = "end";
  re2c:define:YYPEEK   = "(*cursor);";
  re2c:define:YYSKIP   = "(cursor++);";
  re2c:define:YYBACKUP = "(marker = cursor);";
  re2c:define:YYRESTORE = "(cursor = marker);";
  re2c:define:YYLESSTHAN = "(end > (end - cursor));";

  start = "::"?[a-zA-Z_];
  continue = [a-zA-Z0-9_];
  separator = "::";
  id = start continue* (separator continue+)*;

  int = [0-9]+;

  any = ([a-zA-Z0-9~`!@#$%&*_-+=':;?/.,<>|\\\^{}()\[\]] | "\n");
  string = ["] any* ["];
*/
// NOLINTBEGIN(cppcoreguidelines-avoid-goto)
// #REASON: re2c uses gotos to implement the lexer and as all of the
// gotos are from generated code we are trusting re2c to
// use gotos in a safe and sane way here.

auto Scanner::scan() noexcept -> Token {
  while (true) {
    token = cursor;
    /*!re2c
      "nil"     { UpdateLocation(); return Token::Nil; }
      "Nil"     { UpdateLocation(); return Token::NilType; }
      "Integer" { UpdateLocation(); return Token::IntegerType; }
      "true"    { UpdateLocation(); return Token::True; }
      "false"   { UpdateLocation(); return Token::False; }
      "Boolean" { UpdateLocation(); return Token::BooleanType; }

      "let"     { UpdateLocation(); return Token::Let; }
      "module"  { UpdateLocation(); return Token::Module; }
      "public"  { UpdateLocation(); return Token::Public; }
      "private" { UpdateLocation(); return Token::Private; }
      "import"  { UpdateLocation(); return Token::Import; }
      "from"    { UpdateLocation(); return Token::From; }

      "=" { UpdateLocation(); return Token::Equal; }
      ";" { UpdateLocation(); return Token::Semicolon; }
      ":" { UpdateLocation(); return Token::Colon; }
      "," { UpdateLocation(); return Token::Comma; }
      "(" { UpdateLocation(); return Token::BeginParen; }
      ")" { UpdateLocation(); return Token::EndParen; }
      "{" { UpdateLocation(); return Token::BeginBrace; }
      "}" { UpdateLocation(); return Token::EndBrace; }
      "\\" { UpdateLocation(); return Token::BSlash; }
      "->" { UpdateLocation(); return Token::RArrow; }
      "=>" { UpdateLocation(); return Token::EqRArrow; }

      "+" { UpdateLocation(); return Token::Plus; }
      "-" { UpdateLocation(); return Token::Minus; }
      "*" { UpdateLocation(); return Token::Star; }
      "/" { UpdateLocation(); return Token::Divide; }
      "%" { UpdateLocation(); return Token::Modulo; }
      "!" { UpdateLocation(); return Token::Not; }
      "&" { UpdateLocation(); return Token::And; }
      "|" { UpdateLocation(); return Token::Or; }
      "<" { UpdateLocation(); return Token::LessThan; }
      "<=" { UpdateLocation(); return Token::LessThanOrEqual; }
      "==" { UpdateLocation(); return Token::EqualEqual; }
      "!=" { UpdateLocation(); return Token::NotEqual; }
      ">"  { UpdateLocation(); return Token::GreaterThan; }
      ">=" { UpdateLocation(); return Token::GreaterThanOrEqual; }

      id     { UpdateLocation(); return Token::Identifier; }
      int    { UpdateLocation(); return Token::Integer; }
      string { UpdateLocation(); return Token::Text; }

      [ \t\n]+ { UpdateLocation(); continue; } // whitespace
      *        { UpdateLocation(); return Token::Error; } // unknown token
      $        { UpdateLocation(); return Token::End; } // end of input
    */
  }
}
// NOLINTEND(cppcoreguidelines-avoid-goto)
} // namespace mint