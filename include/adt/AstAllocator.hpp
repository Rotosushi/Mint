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
#include <forward_list>
#include <memory>

#include "ast/All.hpp"

namespace mint {
/*
  #TODO: this is a natural place to swap make_unique with allocate_unique.
  as long as we can provide a deleter structure which also references the
  allocator, such that delete works properly.
*/
class AstAllocator {
  std::forward_list<std::unique_ptr<ast::Ast>> m_list;

  template <class T, class... Args>
  [[nodiscard]] auto emplace(Args &&...args) noexcept {
    auto alloc = std::make_unique<T>(std::forward<Args>(args)...);
    auto result = alloc.get();
    m_list.emplace_front(std::move(alloc));
    return result;
  }

public:
  /* Definitions */
  [[nodiscard]] auto createLet(Attributes attributes, Location location,
                               std::optional<type::Ptr> annotation,
                               Identifier name, ast::Ptr ast) noexcept {

    return emplace<ast::Let>(attributes, location, annotation, name, ast);
  }

  /* Expressions */
  [[nodiscard]] auto createBinop(Attributes attributes, Location location,
                                 Token op, ast::Ptr left,
                                 ast::Ptr right) noexcept {
    return emplace<ast::Binop>(attributes, location, op, left, right);
  }

  [[nodiscard]] auto createUnop(Attributes attributes, Location location,
                                Token op, ast::Ptr right) noexcept {
    return emplace<ast::Unop>(attributes, location, op, right);
  }

  [[nodiscard]] auto createVariable(Attributes attributes, Location location,
                                    Identifier name) noexcept {
    return emplace<ast::Variable>(attributes, location, name);
  }

  /* Statements */
  [[nodiscard]] auto createImport(Attributes attributes, Location location,
                                  std::string filename) noexcept {
    return emplace<ast::Import>(attributes, location, std::move(filename));
  }

  [[nodiscard]] auto
  createModule(Attributes attributes, Location location, Identifier name,
               ast::Module::Expressions expressions) noexcept {
    return emplace<ast::Module>(attributes, location, name,
                                std::move(expressions));
  }

  /* Syntax */
  [[nodiscard]] auto createAffix(Attributes attributes, Location location,
                                 ast::Ptr ast) noexcept {
    return emplace<ast::Affix>(attributes, location, ast);
  }

  [[nodiscard]] auto createParens(Attributes attributes, Location location,
                                  ast::Ptr ast) noexcept {
    return emplace<ast::Parens>(attributes, location, ast);
  }

  /* Values */
  [[nodiscard]] auto createBoolean(Attributes attributes, Location location,
                                   bool value) noexcept {
    return emplace<ast::Boolean>(attributes, location, value);
  }

  [[nodiscard]] auto createInteger(Attributes attributes, Location location,
                                   int value) noexcept {
    return emplace<ast::Integer>(attributes, location, value);
  }

  [[nodiscard]] auto createNil(Attributes attributes,
                               Location location) noexcept {
    return emplace<ast::Nil>(attributes, location);
  }
};
} // namespace mint
