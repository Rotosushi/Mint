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
#include "ast/visitors/Print.hpp"

namespace mint::ast {
struct PrintAst {
  std::ostream &out;
  ast::Ptr &ptr;

  PrintAst(std::ostream &out, ast::Ptr &ptr) noexcept : out(out), ptr(ptr) {}

  void operator()() noexcept { std::visit(*this, ptr->variant); }

  void operator()([[maybe_unused]] std::monostate &nil) noexcept {
    out << "nil";
  }

  void operator()(bool &b) noexcept { out << (b ? "true" : "false"); }

  void operator()(int &i) noexcept { out << i; }

  void operator()(Identifier &i) noexcept { out << i; }

  void operator()(Function &f) noexcept {
    if (f.attributes.isPublic()) {
      out << "public ";
    } else {
      out << "private ";
    }

    out << "fn " << f.name;

    out << "(";
    auto index = 0U;
    auto size = f.arguments.size();
    for (auto &arg : f.arguments) {
      out << arg.name << ": " << arg.type;

      if (index++ < (size - 1)) {
        out << ", ";
      }
    }
    out << ")";

    if (f.annotation) {
      out << " -> " << f.annotation.value();
    }

    out << "{";
    for (auto &expression : f.body) {
      out << expression << "\n";
    }
    out << "}";
  }

  void operator()(Let &l) noexcept {
    if (l.attributes.isPublic()) {
      out << "public ";
    } else {
      out << "private ";
    }

    out << "let " << l.name;

    if (l.annotation) {
      out << ": " << l.annotation.value();
    }

    out << " = " << l.affix << ";";
  }

  void operator()(Binop &b) noexcept {
    out << b.left << " " << b.op << " " << b.right;
  }

  void operator()(Unop &u) noexcept { out << u.op << u.right; }

  void operator()(Call &c) noexcept {
    out << c.callee << "(";
    auto index = 0U;
    auto size = c.arguments.size();
    for (auto &arg : c.arguments) {
      out << arg;

      if (index++ < (size - 1)) {
        out << ", ";
      }
    }
    out << ")";
  }

  void operator()(Parens &p) noexcept { out << "(" << p.expression << ")"; }

  void operator()(Import &i) noexcept { out << "import \"" << i.file << "\";"; }

  void operator()(Module &m) noexcept {
    out << "module " << m.name << "{\n";
    for (auto &expression : m.expressions) {
      out << expression << "\n";
    }
    out << "}";
  }
};

void print(std::ostream &out, ast::Ptr &ptr) noexcept {
  PrintAst visitor(out, ptr);
  visitor();
}
} // namespace mint::ast
