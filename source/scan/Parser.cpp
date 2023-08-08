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
#include "scan/Parser.hpp"
#include "adt/Environment.hpp"
#include "ast/All.hpp"
#include "utility/NumbersRoundTrip.hpp"

namespace mint {
Parser::Parser(Environment *env, std::istream *in) noexcept
    : env(env), in(in), current(Token::End) {
  MINT_ASSERT(env != nullptr);
  MINT_ASSERT(in != nullptr);
}

void Parser::setIstream(std::istream *in) noexcept {
  MINT_ASSERT(in != nullptr);
  this->in = in;
}

auto Parser::extractSourceLine(Location const &location) const noexcept
    -> std::string_view {
  auto view = scanner.view();
  auto cursor = view.begin();
  auto end = view.end();
  std::size_t lines_seen = 1;

  while ((lines_seen < location.fline) && (cursor != end)) {
    if (*cursor == '\n')
      lines_seen++;

    cursor++;
  }

  if (cursor != end) {
    auto eol = cursor;

    while (eol != end && *eol != '\n')
      eol++;

    return {cursor, eol};
  }
  return {};
}

auto Parser::endOfInput() const noexcept -> bool {
  return scanner.endOfInput() && in->eof();
}

auto Parser::text() const noexcept -> std::string_view {
  return scanner.getText();
}
auto Parser::location() const noexcept -> Location {
  return scanner.getLocation();
}

void Parser::next() noexcept { current = scanner.scan(); }

void Parser::append(std::string_view text) noexcept { scanner.append(text); }

void Parser::fill() noexcept {
  auto at_end = current == Token::End;
  auto more_source = !in->eof();
  auto in_good = in->good();
  if (at_end && more_source && in_good) {
    std::string line;
    std::getline(*in, line, '\n');
    line.push_back('\n');
    append(line);

    next();
  }
}

auto Parser::peek(Token token) noexcept -> bool {
  fill();
  return current == token;
}

auto Parser::expect(Token token) noexcept -> bool {
  fill();
  if (current == token) {
    next();
    return true;
  }
  return false;
}

auto Parser::predictsDeclaration(Token token) noexcept -> bool {
  fill();
  switch (token) {
  case Token::Let:
  case Token::Module:
    return true;
  default:
    return false;
  }
}

void Parser::recover() noexcept {
  while (!peek(Token::Semicolon) && !peek(Token::End))
    next();

  if (peek(Token::Semicolon))
    next();
}

auto Parser::handle_error(Error::Kind kind) noexcept -> Error {
  return handle_error(kind, location(), text());
}
auto Parser::handle_error(Error::Kind kind, Location location,
                          std::string_view message) noexcept -> Error {
  recover();
  return {kind, location, message};
}

/*
top = visibility? declaration
    | term
*/
auto Parser::parseTop() noexcept -> Result<ast::Ptr> {
  fill();
  if (endOfInput())
    return handle_error(Error::Kind::EndOfInput);

  if (peek(Token::Public)) {
    next();
    return parseDeclaration(/* is_public = */ true);
  } else if (peek(Token::Private)) {
    next();
    return parseDeclaration(/* is_public = */ false);
  } else if (predictsDeclaration(current)) {
    return parseDeclaration(/* is_public = */ false);
  } else if (peek(Token::Import)) {
    return parseImport();
  } else {
    return parseTerm();
  }
}

/*
  declaration = let
              | module
*/
auto Parser::parseDeclaration(bool is_public) noexcept -> Result<ast::Ptr> {
  if (peek(Token::Let))
    return parseLet(is_public);
  else if (peek(Token::Module))
    return parseModule(is_public);
  else
    return handle_error(Error::Kind::ExpectedADeclaration);
}

/*
  let = "let" identifier (":" type)? "=" term
*/
auto Parser::parseLet(bool is_public) noexcept -> Result<ast::Ptr> {
  Attributes attributes = default_attributes;
  attributes.isPublic(is_public);
  std::optional<type::Ptr> annotation;
  auto left_loc = location();
  MINT_ASSERT(peek(Token::Let));
  next(); // eat 'let'

  if (!peek(Token::Identifier))
    return handle_error(Error::Kind::ExpectedAnIdentifier);

  auto id = env->getIdentifier(text());
  next(); // eat identifier

  if (expect(Token::Colon)) {
    auto type = parseType();
    if (!type)
      return type.error();

    annotation = type.value();
  }

  if (!expect(Token::Equal))
    return handle_error(Error::Kind::ExpectedAnEquals);

  auto affix = parseTerm();
  if (!affix)
    return affix;

  auto right_loc = location();
  Location let_loc = {left_loc, right_loc};
  return ast::Let::create(attributes, let_loc, annotation, id,
                          std::move(affix.value()));
}

/*
  module = "module" identifier "{" top* "}"
*/
auto Parser::parseModule(bool is_public) noexcept -> Result<ast::Ptr> {
  Attributes attributes = default_attributes;
  attributes.isPublic(is_public);
  auto left_loc = location();
  /* "module" identifier "{" */
  MINT_ASSERT(peek(Token::Module));
  next(); // eat 'module'

  if (!peek(Token::Identifier))
    return handle_error(Error::Kind::ExpectedAnIdentifier);

  auto id = env->getIdentifier(text());
  next();

  if (!expect(Token::BeginBrace))
    return handle_error(Error::Kind::ExpectedABeginBrace);

  ast::Module::Expressions expressions;
  /* top* '}' */
  while (!expect(Token::EndBrace)) {
    auto expr = parseTop();
    if (!expr)
      return expr;

    expressions.emplace_back(std::move(expr.value()));
  }

  auto right_loc = location();
  Location module_loc = {left_loc, right_loc};
  return ast::Module::create(attributes, module_loc, id,
                             std::move(expressions));
}

/*
  import = "import" string-literal ";"
*/
auto Parser::parseImport() noexcept -> Result<ast::Ptr> {
  auto left_loc = location();
  MINT_ASSERT(peek(Token::Import));
  next(); // eat "import"

  if (!peek(Token::Text))
    return handle_error(Error::Kind::ExpectedText);

  auto file = env->getTextFromTextLiteral(text());
  next(); // eat string

  if (!expect(Token::Semicolon))
    return handle_error(Error::Kind::ExpectedASemicolon);

  auto right_loc = location();
  Location import_loc = {left_loc, right_loc};
  return ast::Import::create(default_attributes, import_loc,
                             env->internString(file));
}

/* term = affix ";" */
auto Parser::parseTerm() noexcept -> Result<ast::Ptr> {
  auto left_loc = location();

  auto result = parseAffix();
  if (!result) {
    return result;
  }
  auto &affix = result.value();

  if (!expect(Token::Semicolon))
    return handle_error(Error::Kind::ExpectedASemicolon);

  auto right_loc = location();
  Location term_loc = {left_loc, right_loc};
  return ast::Affix::create(default_attributes, term_loc, std::move(affix));
}

// affix = call (binop precedence-parser)?
auto Parser::parseAffix() noexcept -> Result<ast::Ptr> {
  auto call = parseCall();
  if (!call)
    return call;

  if (isBinop(current))
    return precedenceParser(std::move(call.value()), 0);

  return call;
}

// call = basic ("(" (affix ("," affix)*)? ")")?
auto Parser::parseCall() noexcept -> Result<ast::Ptr> {
  auto lhs_loc = location();
  auto basic = parseBasic();
  if (!basic)
    return basic;

  // optional call expression
  if (expect(Token::BeginParen)) {
    ast::Call::Arguments arguments;
    // optional arguments to call expression
    if (!peek(Token::EndParen)) {
      do {
        auto result = parseAffix();
        if (!result)
          return result;

        arguments.emplace_back(std::move(result.value()));
      } while (expect(Token::Comma));
    }

    if (!expect(Token::EndParen))
      return handle_error(Error::Kind::ExpectedAClosingParen);

    auto rhs_loc = location();
    Location call_loc = {lhs_loc, rhs_loc};
    basic = ast::Call::create(default_attributes, call_loc,
                              std::move(basic.value()), std::move(arguments));
  }

  return basic;
}

// #TODO: I'm fairly sure that location tracking in
// precedence parsing has a bug in it.
// ... a + b ...
// has location information such that we will highlight
// ... a + b ...
// ...---^^^--...
// instead of the (probably) expected
// ... a + b ...
// ...-^^^^^-...

auto Parser::precedenceParser(ast::Ptr left, BinopPrecedence prec) noexcept
    -> Result<ast::Ptr> {
  Result<ast::Ptr> result = std::move(left);
  Location op_loc;
  Token op{Token::Error};

  auto predicts_binop = [&]() -> bool {
    if (!isBinop(current))
      return false;

    return precedence(current) >= prec;
  };

  auto predictsHigherPrecedenceOrRightAssociativeBinop = [&]() -> bool {
    if (!isBinop(current))
      return false;

    if (precedence(current) > precedence(op))
      return true;

    if ((associativity(op) == BinopAssociativity::Right) &&
        (precedence(current) == precedence(op)))
      return true;

    return false;
  };

  auto new_prec = [&]() -> BinopPrecedence {
    if (precedence(op) > precedence(current))
      return precedence(op) + static_cast<BinopPrecedence>(1);
    else
      return precedence(op);
  };

  while (predicts_binop()) {
    op = current;
    op_loc = location();

    next(); // eat 'op'

    auto right = parseCall();
    if (!right)
      return right;

    while (predictsHigherPrecedenceOrRightAssociativeBinop()) {
      auto temp = precedenceParser(std::move(right.value()), new_prec());
      if (!temp)
        return temp;

      right = std::move(temp);
    }

    auto rhs_loc = right.value()->location();
    Location binop_loc = {op_loc, rhs_loc};
    ast::Ptr &lhs = result.value();
    ast::Ptr &rhs = right.value();
    result = ast::Binop::create(default_attributes, binop_loc, op,
                                std::move(lhs), std::move(rhs));
  }

  return result;
}

/*
basic = "nil"
      | "true"
      | "false"
      | integer
      | identifier
      | unop basic
      | "(" affix ")"
      | "\" (argument-list)? ("->" type)? "=>" affix
*/
auto Parser::parseBasic() noexcept -> Result<ast::Ptr> {
  fill();

  switch (current) {
  case Token::Nil:
    return parseNil();

  case Token::True:
    return parseTrue();

  case Token::False:
    return parseFalse();

  case Token::Integer:
    return parseInteger();

  case Token::Identifier:
    return parseVariable();

  case Token::Not:
  case Token::Minus:
    return parseUnop();

  case Token::BeginParen:
    return parseParens();

  case Token::BSlash:
    return parseLambda();

  default:
    return handle_error(Error::Kind::ExpectedABasicTerm);
  }
}

auto Parser::parseNil() noexcept -> Result<ast::Ptr> {
  auto loc = location();
  next();
  return ast::Nil::create(default_attributes, loc);
}

auto Parser::parseTrue() noexcept -> Result<ast::Ptr> {
  auto loc = location();
  next();
  return ast::Boolean::create(default_attributes, loc, true);
}

auto Parser::parseFalse() noexcept -> Result<ast::Ptr> {
  auto loc = location();
  next();
  return ast::Boolean::create(default_attributes, loc, false);
}

auto Parser::parseInteger() noexcept -> Result<ast::Ptr> {
  int value = fromString<int>(text());
  auto loc = location();
  next();
  return ast::Integer::create(default_attributes, loc, value);
}

auto Parser::parseVariable() noexcept -> Result<ast::Ptr> {
  auto name = env->getIdentifier(text());
  auto loc = location();
  next(); // eat 'id'
  return ast::Variable::create(default_attributes, loc, name);
}

auto Parser::parseUnop() noexcept -> Result<ast::Ptr> {
  auto lhs_loc = location();
  auto op = current;
  next(); // eat unop

  auto right = parseBasic();
  if (!right)
    return right;
  auto &ast = right.value();

  auto rhs_loc = location();
  Location unop_loc{lhs_loc, rhs_loc};
  return ast::Unop::create(default_attributes, unop_loc, op, std::move(ast));
}

auto Parser::parseParens() noexcept -> Result<ast::Ptr> {
  next(); // eat '('

  auto affix = parseAffix();
  if (!affix)
    return affix;
  auto &ast = affix.value();

  if (!expect(Token::EndParen))
    return handle_error(Error::Kind::ExpectedAClosingParen);

  auto loc = ast->location();
  return ast::Parens::create(default_attributes, loc, std::move(ast));
}

auto Parser::parseLambda() noexcept -> Result<ast::Ptr> {
  auto lhs_loc = location();
  next(); // eat '\'
  ast::Lambda::Arguments arguments;
  type::Ptr result_type{nullptr};

  auto parseArgument = [&]() -> Result<ast::Lambda::Argument> {
    if (!peek(Token::Identifier))
      return handle_error(Error::Kind::ExpectedAnIdentifier);

    auto name = env->getIdentifier(text());
    next();

    if (!expect(Token::Colon))
      return handle_error(Error::Kind::ExpectedAColon);

    auto result = parseType();
    if (!result)
      return result.error();
    auto type = result.value();
    return ast::Lambda::Argument{name, default_attributes, type};
  };

  auto parseArguments = [&]() -> Result<ast::Lambda::Arguments> {
    ast::Lambda::Arguments arguments;
    do {
      auto result = parseArgument();
      if (!result)
        return result.error();

      arguments.emplace_back(result.value());
    } while (expect(Token::Comma));
    return arguments;
  };

  // optional argument list
  if (peek(Token::Identifier)) {
    auto result = parseArguments();
    if (!result)
      return result.error();

    arguments = std::move(result.value());
  }

  // optional type annotation
  if (expect(Token::RArrow)) { // eat '->'
    auto result = parseType();
    if (!result)
      return result.error();

    result_type = result.value();
  }

  // parse the body
  if (!expect(Token::EqRArrow)) // eat '=>'
    return handle_error(Error::Kind::ExpectedAEqualsRightArrow);

  auto result = parseAffix();
  if (!result)
    return result;
  auto &body = result.value();

  auto rhs_loc = location();
  Location lambda_loc{lhs_loc, rhs_loc};
  return ast::Lambda::create(default_attributes, lambda_loc,
                             std::move(arguments), result_type,
                             std::move(body));
}

auto Parser::parseType() noexcept -> Result<type::Ptr> {
  fill();

  switch (current) {
  case Token::NilType:
    return parseNilType();

  case Token::BooleanType:
    return parseBooleanType();

  case Token::IntegerType:
    return parseIntegerType();

  case Token::BSlash:
    return parseFunctionType();

  default:
    return handle_error(Error::Kind::ExpectedAType);
  }
}

auto Parser::parseNilType() noexcept -> Result<type::Ptr> {
  next();
  return env->getNilType();
}

auto Parser::parseBooleanType() noexcept -> Result<type::Ptr> {
  next();
  return env->getBooleanType();
}

auto Parser::parseIntegerType() noexcept -> Result<type::Ptr> {
  next();
  return env->getIntegerType();
}

auto Parser::parseFunctionType() noexcept -> Result<type::Ptr> {
  next(); // eat '\'
  std::vector<type::Ptr> arguments;

  // parse the argument types
  if (!peek(Token::RArrow))
    do {
      auto result = parseType();
      if (!result)
        return result;

      arguments.push_back(result.value());
    } while (expect(Token::Comma)); // eat ','

  if (!expect(Token::RArrow)) // eat '->'
    return handle_error(Error::Kind::ExpectedARightArrow);

  auto result = parseType();
  if (!result)
    return result;

  return env->getFunctionType(result.value(), std::move(arguments));
}

} // namespace mint
