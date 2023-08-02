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
#include "ast/value/Boolean.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace ast {
Boolean::Boolean(Attributes attributes, Location location, bool value) noexcept
    : Value{Ast::Kind::Boolean, attributes, location}, m_value{value} {}

[[nodiscard]] auto Boolean::create(Attributes attributes, Location location,
                                   bool value) noexcept -> ast::Ptr {
  return static_cast<std::unique_ptr<Ast>>(
      std::make_unique<Boolean>(attributes, location, value));
}

auto Boolean::classof(Ast const *ast) noexcept -> bool {
  return ast->kind() == Ast::Kind::Boolean;
}

auto Boolean::value() const noexcept -> bool { return m_value; }

void Boolean::print(std::ostream &out) const noexcept {
  out << (m_value ? "true" : "false");
}

Ptr Boolean::clone() const noexcept {
  return create(attributes(), location(), m_value);
}

Result<type::Ptr> Boolean::typecheck(Environment &env) const noexcept {
  setCachedType(env.getBooleanType());
  return env.getBooleanType();
}

Result<ast::Ptr> Boolean::evaluate([[maybe_unused]] Environment &env) noexcept {
  return clone();
}

Result<llvm::Value *> Boolean::codegen(Environment &env) noexcept {
  return env.getLLVMBoolean(m_value);
}
} // namespace ast
} // namespace mint
