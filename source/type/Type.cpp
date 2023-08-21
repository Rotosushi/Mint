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
#include "type/Type.hpp"
#include "adt/Environment.hpp"

namespace mint {
namespace type {
struct TypeEqualsVisitor {
  Ptr left;
  Ptr right;

  TypeEqualsVisitor(Ptr left, Ptr right) noexcept : left(left), right(right) {}

  bool operator()() noexcept { return std::visit(*this, left->variant); }

  bool operator()([[maybe_unused]] Nil &nil) noexcept {
    return std::holds_alternative<Nil>(right->variant);
  }

  bool operator()([[maybe_unused]] Boolean &boolean) noexcept {
    return std::holds_alternative<Boolean>(right->variant);
  }

  bool operator()([[maybe_unused]] Integer &integer) noexcept {
    return std::holds_alternative<Integer>(right->variant);
  }

  bool operator()(Function &function) noexcept {
    if (!std::holds_alternative<Function>(right->variant))
      return false;

    auto &other = std::get<Function>(right->variant);

    if (function.arguments.size() != other.arguments.size())
      return false;

    auto cursor = function.arguments.begin();
    auto end = function.arguments.end();
    auto other_cursor = other.arguments.begin();
    while (cursor != end) {
      if (!equals(*cursor, *other_cursor))
        return false;
      ++cursor;
      ++other_cursor;
    }

    return equals(function.result_type, other.result_type);
  }

  bool operator()(Lambda &lambda) noexcept {
    if (!std::holds_alternative<Lambda>(right->variant))
      return false;

    auto &other = std::get<Lambda>(right->variant);

    return equals(lambda.function_type, other.function_type);
  }
};

bool equals(Ptr left, Ptr right) noexcept {
  TypeEqualsVisitor visitor(left, right);
  return visitor();
}

struct TypeCallableVisitor {
  bool operator()(Ptr type) noexcept {
    return std::visit(*this, type->variant);
  }

  bool operator()([[maybe_unused]] Nil &nil) noexcept { return false; }
  bool operator()([[maybe_unused]] Boolean &nil) noexcept { return false; }
  bool operator()([[maybe_unused]] Integer &nil) noexcept { return false; }
  bool operator()([[maybe_unused]] Function &nil) noexcept { return true; }
  bool operator()([[maybe_unused]] Lambda &nil) noexcept { return true; }
};

bool callable(Ptr type) noexcept {
  TypeCallableVisitor visitor;
  return visitor(type);
}

struct TypePrintVisitor {
  std::ostream &out;

  TypePrintVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(Ptr type) noexcept { std::visit(*this, type->variant); }

  void operator()([[maybe_unused]] Nil &nil) noexcept { out << "Nil"; }

  void operator()([[maybe_unused]] Boolean &boolean) noexcept {
    out << "Boolean";
  }

  void operator()([[maybe_unused]] Integer &integer) noexcept {
    out << "Integer";
  }

  void operator()(Function &function) noexcept {
    out << "\\";

    auto size = function.arguments.size();
    auto index = 0U;
    for (auto type : function.arguments) {
      out << type;

      if (index++ < (size - 1))
        out << ", ";
    }

    out << " -> " << function.result_type;
  }

  void operator()(Lambda &lambda) noexcept { out << lambda.function_type; }
};

void print(std::ostream &out, Ptr type) noexcept {
  TypePrintVisitor visitor(out);
  return visitor(type);
}

struct TypeToLLVMVisitor {
  Environment &env;

  TypeToLLVMVisitor(Environment &env) noexcept : env(env) {}

  llvm::Type *operator()(Ptr type) noexcept {
    return std::visit(*this, type->variant);
  }

  llvm::Type *operator()([[maybe_unused]] Nil &nil) noexcept {
    return env.getLLVMNilType();
  }

  llvm::Type *operator()([[maybe_unused]] Boolean &boolean) noexcept {
    return env.getLLVMBooleanType();
  }

  llvm::Type *operator()([[maybe_unused]] Integer &integer) noexcept {
    return env.getLLVMIntegerType();
  }

  llvm::Type *operator()(Function &function) noexcept {
    std::vector<llvm::Type *> arguments;
    arguments.reserve(function.arguments.size());
    for (auto type : function.arguments) {
      arguments.emplace_back(toLLVM(type, env));
    }

    return env.getLLVMFunctionType(toLLVM(function.result_type, env),
                                   std::move(arguments));
  }

  llvm::Type *operator()([[maybe_unused]] Lambda &lambda) noexcept {
    return env.getLLVMPointerType();
  }
};

llvm::Type *toLLVM(Ptr type, Environment &env) noexcept {
  TypeToLLVMVisitor visitor(env);
  return visitor(type);
}
} // namespace type
} // namespace mint
