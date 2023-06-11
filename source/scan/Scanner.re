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
  id= start continue* (separator continue+)*;
  int=[0-9]+;
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

      "let"    { UpdateLocation(); return Token::Let; }
      "module" { UpdateLocation(); return Token::Module; }

      "=" { UpdateLocation(); return Token::Equal; }
      ";" { UpdateLocation(); return Token::Semicolon; }
      "(" { UpdateLocation(); return Token::BeginParen; }
      ")" { UpdateLocation(); return Token::EndParen; }
      "{" { UpdateLocation(); return Token::BeginBrace; }
      "}" { UpdateLocation(); return Token::EndBrace; }

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

      id  { UpdateLocation(); return Token::Identifier; }
      int { UpdateLocation(); return Token::Integer; }

      [ \t\n]+ { UpdateLocation(); continue; } // whitespace
      *        { UpdateLocation(); return Token::Error; } // unknown token
      $        { UpdateLocation(); return Token::End; } // end of input
    */
  }
}
// NOLINTEND(cppcoreguidelines-avoid-goto)
} // namespace mint