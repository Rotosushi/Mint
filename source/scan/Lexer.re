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
#include "scan/Lexer.hpp"

namespace mint {
/*!re2c
  re2c:api = custom;
  re2c:api:style = free-form;
  re2c:yyfill:enable = 0;
  re2c:eof = 0;

  re2c:define:YYCTYPE  = "char";
  re2c:define:YYCURSOR = "cursor()";
  re2c:define:YYMARKER = "marker()";
  re2c:define:YYLIMIT = "end()";
  re2c:define:YYPEEK   = "peek()";
  re2c:define:YYSKIP   = "skip();";
  re2c:define:YYBACKUP = "backup();";
  re2c:define:YYRESTORE = "restore();";
  re2c:define:YYLESSTHAN = "endOfInput()";

  start = "::"?[a-zA-Z_];
  continue = [a-zA-Z0-9_];
  separator = "::";
  id = start continue* (separator continue+)*;

  int = [0-9]+;

  any = ([a-zA-Z0-9~`!@#$%&*_-+=':;?/.,<>|\\\^{}()\[\]] | "\n");
  string = ["] any* ["];
*/
// NOLINTBEGIN(cppcoreguidelines-avoid-goto)
Token Lexer::lex() noexcept {
  while (true) {
    prime();
    /*!re2c
      [ \t\n]+ { updateCurrentLocation(); continue; }
      *        { updateCurrentLocation(); return Token::Error; }
      $        { updateCurrentLocation(); return Token::End; }

      "nil"     { updateCurrentLocation(); return Token::Nil; }
      "Nil"     { updateCurrentLocation(); return Token::NilType; }
      "Integer" { updateCurrentLocation(); return Token::IntegerType; }
      "true"    { updateCurrentLocation(); return Token::True; }
      "false"   { updateCurrentLocation(); return Token::False; }
      "Boolean" { updateCurrentLocation(); return Token::BooleanType; }

      "let"     { updateCurrentLocation(); return Token::Let; }
      "module"  { updateCurrentLocation(); return Token::Module; }
      "public"  { updateCurrentLocation(); return Token::Public; }
      "private" { updateCurrentLocation(); return Token::Private; }
      "import"  { updateCurrentLocation(); return Token::Import; }
      "from"    { updateCurrentLocation(); return Token::From; }

      "=" { updateCurrentLocation(); return Token::Equal; }
      ";" { updateCurrentLocation(); return Token::Semicolon; }
      ":" { updateCurrentLocation(); return Token::Colon; }
      "," { updateCurrentLocation(); return Token::Comma; }
      "(" { updateCurrentLocation(); return Token::BeginParen; }
      ")" { updateCurrentLocation(); return Token::EndParen; }
      "{" { updateCurrentLocation(); return Token::BeginBrace; }
      "}" { updateCurrentLocation(); return Token::EndBrace; }
      "\\" { updateCurrentLocation(); return Token::BackSlash; }
      "->" { updateCurrentLocation(); return Token::RightArrow; }
      "=>" { updateCurrentLocation(); return Token::EqualsRightArrow; }

      "+" { updateCurrentLocation(); return Token::Plus; }
      "-" { updateCurrentLocation(); return Token::Minus; }
      "*" { updateCurrentLocation(); return Token::Star; }
      "/" { updateCurrentLocation(); return Token::Divide; }
      "%" { updateCurrentLocation(); return Token::Modulo; }
      "!" { updateCurrentLocation(); return Token::Not; }
      "&" { updateCurrentLocation(); return Token::And; }
      "|" { updateCurrentLocation(); return Token::Or; }
      "<" { updateCurrentLocation(); return Token::LessThan; }
      "<=" { updateCurrentLocation(); return Token::LessThanOrEqual; }
      "==" { updateCurrentLocation(); return Token::EqualEqual; }
      "!=" { updateCurrentLocation(); return Token::NotEqual; }
      ">"  { updateCurrentLocation(); return Token::GreaterThan; }
      ">=" { updateCurrentLocation(); return Token::GreaterThanOrEqual; }

      id     { updateCurrentLocation(); return Token::Identifier; }
      int    { updateCurrentLocation(); return Token::Integer; }
      string { updateCurrentLocation(); return Token::Text; }
    */
  }
}
// NOLINTEND(cppcoreguidelines-avoid-goto)
} // namespace mint
