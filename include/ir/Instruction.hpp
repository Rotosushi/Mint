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
#include "ir/All.hpp"

#include "utility/Abort.hpp"

namespace mint {
namespace ir {
// #TODO: perhaps 'Instruction' isn't the best name?
// the instention is simply the variant representing
// one of the available actions within the IR, that is
// an element of the Array representing a given Ast.
// Node? Element?
// I was sort of planning on using Mir to mean the entire
// flattened Ast. but maybe Mir fits better here? idk
class Instruction {
public:
  enum Kind {
    Let,

    Binop,
    Call,
    Unop,

    Import,
    Module,

    Lambda,
    // #NOTE: Scalar and Parameter are
    // elements of an instruction, not
    // instructions themselves.
  };

private:
  Kind m_kind;

  union Variant {
    ir::Let m_let;

    ir::Binop m_binop;
    ir::Call m_call;
    ir::Unop m_unop;

    ir::Import m_import;

    ir::Lambda m_lambda;

    Variant(Identifier name, Parameter parameter) noexcept
        : m_let(name, parameter) {}
    Variant(ir::Binop::Op op, Parameter left, Parameter right) noexcept
        : m_binop(op, left, right) {}
    Variant(Parameter callee, std::vector<Parameter> arguments) noexcept
        : m_call(callee, std::move(arguments)) {}
    Variant(ir::Unop::Op op, Parameter right) noexcept : m_unop(op, right) {}
    Variant(std::string_view file) noexcept : m_import(file) {}
    Variant(std::vector<Lambda::Argument> arguments, Parameter body,
            type::Ptr result_type) noexcept
        : m_lambda(std::move(arguments), body, result_type) {}
  };
  Variant m_variant;

  void assign(Kind kind, Variant &left, Variant &right) noexcept {
    switch (kind) {
    case Kind::Let:
      left.m_variant.m_let = right.m_variant.m_let;
      break;

    case Kind::Binop:
      left.m_variant.m_binop = right.m_variant.m_binop;
      break;
    case Kind::Call:
      left.m_variant.m_call = right.m_variant.m_call;
      break;
    case Kind::Unop:
      left.m_variant.m_unop = right.m_variant.m_unop;
      break;

    case Kind::Import:
      left.m_variant.m_import = right.m_variant.m_import;
      break;

    case Kind::Lambda:
      left.m_variant.m_lambda = right.m_variant.m_lambda;
      break;

    default:
      abort("bad Instruction Variant Kind");
    }
  }

public:
  Instruction(Identifier name, Parameter parameter) noexcept
      : m_kind(Let), m_variant(name, parameter) {}
  Instruction(ir::Binop::Op op, Parameter left, Parameter right) noexcept
      : m_kind(Binop), m_variant(op, left, right) {}
  Instruction(Parameter callee, std::vector<Parameter> arguments) noexcept
      : m_kind(Call), m_variant(callee, std::move(arguments)) {}
  Instruction(ir::Unop::Op op, Parameter right) noexcept
      : m_kind(Unop), m_variant(op, right) {}
  Instruction(std::string_view file) noexcept
      : m_kind(Import), m_variant(file) {}
  Instruction(std::vector<Lambda::Argument> arguments, Parameter body,
              type::Ptr result_type) noexcept
      : m_kind(Lambda), m_variant(std::move(arguments), body, result_type) {}
  Instruction(Instruction const &other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  Instruction(Instruction &&other) noexcept : m_kind(other.m_kind) {
    assign(kind(), m_variant, other.m_variant);
  }
  auto operator=(Instruction const &other) noexcept -> Instruction & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
    return *this;
  }
  auto operator=(Instruction &&other) noexcept -> Instruction & {
    if (this == &other)
      return *this;

    m_kind = other.m_kind;
    assign(kind(), m_variant, other.m_variant);
    return *this;
  }
  ~Instruction() noexcept = default;

  [[nodiscard]] auto kind() const noexcept -> Kind { return m_kind; }

  [[nodiscard]] auto let() const noexcept -> ir::Let & {
    MINT_ASSERT(kind() == Let);
    return m_variant.m_let;
  }

  [[nodiscard]] auto binop() const noexcept -> ir::Binop & {
    MINT_ASSERT(kind() == Binop);
    return m_variant.m_binop;
  }

  [[nodiscard]] auto call() const noexcept -> ir::Call & {
    MINT_ASSERT(kind() == Call);
    return m_variant.m_call;
  }

  [[nodiscard]] auto unop() const noexcept -> ir::Unop & {
    MINT_ASSERT(kind() == Unop);
    return m_variant.m_Unop;
  }

  [[nodiscard]] auto import_() const noexcept -> ir::Import & {
    MINT_ASSERT(kind() == Import);
    return m_variant.m_import;
  }

  [[nodiscard]] auto lambda() const noexcept -> ir::Lambda & {
    MINT_ASSERT(kind() == Lambda);
    return m_variant.m_lambda;
  }
};
} // namespace ir

} // namespace mint
