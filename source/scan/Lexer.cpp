/* Generated by re2c 3.0 on Wed Aug 23 21:08:06 2023 */
#line 1 "/home/cadence/projects/Mint/source/scan/Lexer.re"
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
#line 45 "/home/cadence/projects/Mint/source/scan/Lexer.re"

// NOLINTBEGIN(cppcoreguidelines-avoid-goto)
Token Lexer::lex() noexcept {
  while (true) {
    prime();
    
#line 30 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
{
	char yych;
	unsigned int yyaccept = 0;
	yych = peek();
	switch (yych) {
		case '\t':
		case '\n':
		case ' ': goto yy3;
		case '!': goto yy5;
		case '"': goto yy7;
		case '%': goto yy8;
		case '&': goto yy9;
		case '(': goto yy10;
		case ')': goto yy11;
		case '*': goto yy12;
		case '+': goto yy13;
		case ',': goto yy14;
		case '-': goto yy15;
		case '/': goto yy17;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy18;
		case ':': goto yy20;
		case ';': goto yy22;
		case '<': goto yy23;
		case '=': goto yy25;
		case '>': goto yy27;
		case 'A':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'g':
		case 'h':
		case 'j':
		case 'k':
		case 'o':
		case 'q':
		case 'r':
		case 's':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy29;
		case 'B': goto yy32;
		case 'I': goto yy33;
		case 'N': goto yy34;
		case '\\': goto yy35;
		case 'f': goto yy36;
		case 'i': goto yy37;
		case 'l': goto yy38;
		case 'm': goto yy39;
		case 'n': goto yy40;
		case 'p': goto yy41;
		case 't': goto yy42;
		case '{': goto yy43;
		case '|': goto yy44;
		case '}': goto yy45;
		default:
			if (endOfInput()) goto yy120;
			goto yy1;
	}
yy1:
	skip();
yy2:
#line 97 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Error; }
#line 131 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy3:
	skip();
	yych = peek();
	switch (yych) {
		case '\t':
		case '\n':
		case ' ': goto yy3;
		default: goto yy4;
	}
yy4:
#line 96 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); continue; }
#line 144 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy5:
	skip();
	yych = peek();
	switch (yych) {
		case '=': goto yy46;
		default: goto yy6;
	}
yy6:
#line 82 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Not; }
#line 155 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy7:
	yyaccept = 0;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '\n':
		case '!':
		case '"':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '.':
		case '/':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '_':
		case '`':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case '{':
		case '|':
		case '}':
		case '~': goto yy48;
		default: goto yy2;
	}
yy8:
	skip();
#line 81 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Modulo; }
#line 263 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy9:
	skip();
#line 83 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::And; }
#line 268 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy10:
	skip();
#line 69 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::BeginParen; }
#line 273 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy11:
	skip();
#line 70 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::EndParen; }
#line 278 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy12:
	skip();
#line 79 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Star; }
#line 283 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy13:
	skip();
#line 77 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Plus; }
#line 288 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy14:
	skip();
#line 68 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Comma; }
#line 293 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy15:
	skip();
	yych = peek();
	switch (yych) {
		case '>': goto yy51;
		default: goto yy16;
	}
yy16:
#line 78 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Minus; }
#line 304 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy17:
	skip();
#line 80 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Divide; }
#line 309 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy18:
	skip();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy18;
		default: goto yy19;
	}
yy19:
#line 93 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Integer; }
#line 329 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy20:
	yyaccept = 1;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case ':': goto yy52;
		default: goto yy21;
	}
yy21:
#line 67 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Colon; }
#line 342 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy22:
	skip();
#line 66 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Semicolon; }
#line 347 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy23:
	skip();
	yych = peek();
	switch (yych) {
		case '=': goto yy53;
		default: goto yy24;
	}
yy24:
#line 85 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::LessThan; }
#line 358 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy25:
	skip();
	yych = peek();
	switch (yych) {
		case '=': goto yy54;
		case '>': goto yy55;
		default: goto yy26;
	}
yy26:
#line 65 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Equal; }
#line 370 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy27:
	skip();
	yych = peek();
	switch (yych) {
		case '=': goto yy56;
		default: goto yy28;
	}
yy28:
#line 89 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::GreaterThan; }
#line 381 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy29:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
yy30:
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy29;
		case ':': goto yy57;
		default: goto yy31;
	}
yy31:
#line 92 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Identifier; }
#line 458 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy32:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'o': goto yy58;
		default: goto yy30;
	}
yy33:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'n': goto yy59;
		default: goto yy30;
	}
yy34:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'i': goto yy60;
		default: goto yy30;
	}
yy35:
	skip();
#line 73 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::BSlash; }
#line 493 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy36:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'a': goto yy61;
		case 'r': goto yy62;
		default: goto yy30;
	}
yy37:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'm': goto yy63;
		default: goto yy30;
	}
yy38:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy64;
		default: goto yy30;
	}
yy39:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'o': goto yy65;
		default: goto yy30;
	}
yy40:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'i': goto yy66;
		default: goto yy30;
	}
yy41:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'r': goto yy67;
		case 'u': goto yy68;
		default: goto yy30;
	}
yy42:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'r': goto yy69;
		default: goto yy30;
	}
yy43:
	skip();
#line 71 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::BeginBrace; }
#line 570 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy44:
	skip();
#line 84 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Or; }
#line 575 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy45:
	skip();
#line 72 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::EndBrace; }
#line 580 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy46:
	skip();
#line 88 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::NotEqual; }
#line 585 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy47:
	skip();
	yych = peek();
yy48:
	switch (yych) {
		case '\n':
		case '!':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '.':
		case '/':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '_':
		case '`':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case '{':
		case '|':
		case '}':
		case '~': goto yy47;
		case '"': goto yy50;
		default: goto yy49;
	}
yy49:
	restore();
	switch (yyaccept) {
		case 0: goto yy2;
		case 1: goto yy21;
		case 2: goto yy31;
		case 3: goto yy74;
		case 4: goto yy79;
		case 5: goto yy82;
		case 6: goto yy90;
		case 7: goto yy96;
		case 8: goto yy100;
		case 9: goto yy108;
		case 10: goto yy110;
		case 11: goto yy113;
		case 12: goto yy115;
		case 13: goto yy117;
		default: goto yy119;
	}
yy50:
	skip();
#line 94 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Text; }
#line 711 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy51:
	skip();
#line 74 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::RArrow; }
#line 716 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy52:
	skip();
	yych = peek();
	switch (yych) {
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy29;
		default: goto yy49;
	}
yy53:
	skip();
#line 86 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::LessThanOrEqual; }
#line 780 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy54:
	skip();
#line 87 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::EqualEqual; }
#line 785 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy55:
	skip();
#line 75 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::EqRArrow; }
#line 790 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy56:
	skip();
#line 90 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::GreaterThanOrEqual; }
#line 795 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy57:
	skip();
	yych = peek();
	switch (yych) {
		case ':': goto yy70;
		default: goto yy49;
	}
yy58:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'o': goto yy71;
		default: goto yy30;
	}
yy59:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 't': goto yy72;
		default: goto yy30;
	}
yy60:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy73;
		default: goto yy30;
	}
yy61:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy75;
		default: goto yy30;
	}
yy62:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'o': goto yy76;
		default: goto yy30;
	}
yy63:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'p': goto yy77;
		default: goto yy30;
	}
yy64:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 't': goto yy78;
		default: goto yy30;
	}
yy65:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'd': goto yy80;
		default: goto yy30;
	}
yy66:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy81;
		default: goto yy30;
	}
yy67:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'i': goto yy83;
		default: goto yy30;
	}
yy68:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'b': goto yy84;
		default: goto yy30;
	}
yy69:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'u': goto yy85;
		default: goto yy30;
	}
yy70:
	skip();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy29;
		default: goto yy49;
	}
yy71:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy86;
		default: goto yy30;
	}
yy72:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy87;
		default: goto yy30;
	}
yy73:
	yyaccept = 3;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy74;
	}
yy74:
#line 52 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::NilType; }
#line 1087 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy75:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 's': goto yy88;
		default: goto yy30;
	}
yy76:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'm': goto yy89;
		default: goto yy30;
	}
yy77:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'o': goto yy91;
		default: goto yy30;
	}
yy78:
	yyaccept = 4;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy79;
	}
yy79:
#line 58 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Let; }
#line 1193 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy80:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'u': goto yy92;
		default: goto yy30;
	}
yy81:
	yyaccept = 5;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy82;
	}
yy82:
#line 51 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Nil; }
#line 1279 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy83:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'v': goto yy93;
		default: goto yy30;
	}
yy84:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy94;
		default: goto yy30;
	}
yy85:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy95;
		default: goto yy30;
	}
yy86:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy97;
		default: goto yy30;
	}
yy87:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'g': goto yy98;
		default: goto yy30;
	}
yy88:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy99;
		default: goto yy30;
	}
yy89:
	yyaccept = 6;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy90;
	}
yy90:
#line 63 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::From; }
#line 1415 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy91:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'r': goto yy101;
		default: goto yy30;
	}
yy92:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'l': goto yy102;
		default: goto yy30;
	}
yy93:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'a': goto yy103;
		default: goto yy30;
	}
yy94:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'i': goto yy104;
		default: goto yy30;
	}
yy95:
	yyaccept = 7;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy96;
	}
yy96:
#line 54 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::True; }
#line 1531 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy97:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'a': goto yy105;
		default: goto yy30;
	}
yy98:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy106;
		default: goto yy30;
	}
yy99:
	yyaccept = 8;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy100;
	}
yy100:
#line 55 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::False; }
#line 1627 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy101:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 't': goto yy107;
		default: goto yy30;
	}
yy102:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy109;
		default: goto yy30;
	}
yy103:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 't': goto yy111;
		default: goto yy30;
	}
yy104:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'c': goto yy112;
		default: goto yy30;
	}
yy105:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'n': goto yy114;
		default: goto yy30;
	}
yy106:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'r': goto yy116;
		default: goto yy30;
	}
yy107:
	yyaccept = 9;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy108;
	}
yy108:
#line 62 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Import; }
#line 1763 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy109:
	yyaccept = 10;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy110;
	}
yy110:
#line 59 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Module; }
#line 1839 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy111:
	yyaccept = 2;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case 0x00: goto yy31;
		case 'e': goto yy118;
		default: goto yy30;
	}
yy112:
	yyaccept = 11;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy113;
	}
yy113:
#line 60 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Public; }
#line 1925 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy114:
	yyaccept = 12;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy115;
	}
yy115:
#line 56 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::BooleanType; }
#line 2001 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy116:
	yyaccept = 13;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy117;
	}
yy117:
#line 53 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::IntegerType; }
#line 2077 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy118:
	yyaccept = 14;
	skip();
	backup();
	yych = peek();
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case ':':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy30;
		default: goto yy119;
	}
yy119:
#line 61 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::Private; }
#line 2153 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
yy120:
#line 98 "/home/cadence/projects/Mint/source/scan/Lexer.re"
	{ updateCurrentLocation(); return Token::End; }
#line 2157 "/home/cadence/projects/Mint/source/scan/Lexer.cpp"
}
#line 99 "/home/cadence/projects/Mint/source/scan/Lexer.re"

  }
}
// NOLINTEND(cppcoreguidelines-avoid-goto)
} // namespace mint
