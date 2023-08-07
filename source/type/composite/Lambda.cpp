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
#include "type/composite/Lambda.hpp"
#include "adt/Environment.hpp"

#include <algorithm>

namespace mint {
namespace type {
Lambda::Lambda(type::Ptr result_type) noexcept
    : Type(Type::Kind::Function), m_result_type(result_type), m_arguments() {}

Lambda::Lambda(type::Ptr result_type, Arguments arguments) noexcept
    : Type(Type::Kind::Function), m_result_type(result_type),
      m_arguments(std::move(arguments)) {}

auto Lambda::classof(type::Ptr type) noexcept -> bool {
  return Type::Kind::Function == type->kind();
}

[[nodiscard]] auto Lambda::result_type() const noexcept -> type::Ptr {
  return m_result_type;
}

[[nodiscard]] auto Lambda::arguments() const noexcept -> Arguments const & {
  return m_arguments;
}

[[nodiscard]] bool Lambda::equals(type::Ptr type) const noexcept {
  if (auto function = llvm::dyn_cast<Lambda>(type); function != nullptr) {
    if (!m_result_type->equals(function->m_result_type))
      return false;

    if (m_arguments.size() != function->m_arguments.size())
      return false;

    auto pair = std::mismatch(m_arguments.begin(), m_arguments.end(),
                              function->m_arguments.begin());
    return pair.first == m_arguments.end();
  }
  return false;
}

void Lambda::print(std::ostream &out) const noexcept {
  out << "\\";

  auto size = m_arguments.size();
  auto index = 0U;
  for (auto type : m_arguments) {
    out << type;

    if (index++ < (size - 1))
      out << ", ";
  }

  out << " -> " << m_result_type;
}

[[nodiscard]] llvm::Type *Lambda::toLLVMImpl(Environment &env) const noexcept {
  auto llvm_result_type = m_result_type->toLLVM(env);
  std::vector<llvm::Type *> llvm_argument_types;

  if (m_arguments.size() > 0) {
    llvm_argument_types.reserve(m_arguments.size());

    for (auto type : m_arguments)
      llvm_argument_types.push_back(type->toLLVM(env));
  }

  return cachedType(
      env.getLLVMFunctionType(llvm_result_type, llvm_argument_types));
}
} // namespace type
} // namespace mint
