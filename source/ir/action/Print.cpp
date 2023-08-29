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
#include "ir/action/Print.hpp"
#include "ir/Instruction.hpp"

namespace mint::ir {
struct PrintScalarVisitor {
  std::ostream &out;
  PrintScalarVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(Scalar &scalar) noexcept {
    std::visit(*this, scalar.variant());
  }

  void operator()([[maybe_unused]] std::monostate &nil) noexcept {
    out << "nil";
  }

  void operator()(bool &boolean) noexcept {
    out << (boolean ? "true" : "false");
  }

  void operator()(int integer) noexcept { out << integer; }
};

void print(std::ostream &out, Scalar &scalar) {
  PrintScalarVisitor visitor(out);
  visitor(scalar);
}

struct PrintImmediateVisitor {
  std::ostream &out;
  PrintImmediateVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(detail::Immediate &immediate) noexcept {
    std::visit(*this, immediate.variant());
  }

  void operator()(Scalar &scalar) noexcept { print(out, scalar); }

  void operator()(Identifier &name) noexcept { out << name; }
};

void print(std::ostream &out, detail::Immediate &immediate) noexcept {
  PrintImmediateVisitor visitor(out);
  visitor(immediate);
}

void print(std::ostream &out, Mir &mir, detail::Index index) noexcept;

void print(std::ostream &out, Mir &mir, detail::Parameter &parameter) noexcept {
  print(out, mir, parameter.index());
}

struct PrintInstructionVisitor {
  std::ostream &out;
  Mir &mir;
  PrintInstructionVisitor(std::ostream &out, Mir &mir) noexcept
      : out(out), mir(mir) {}

  void operator()(detail::Index index) {
    std::visit(*this, mir[index].variant());
  }

  void operator()(detail::Immediate &immediate) noexcept {
    print(out, immediate);
  }

  void operator()(Parens &parens) noexcept {
    out << "(";
    print(out, mir, parens.parameter());
    out << ")";
  }

  void operator()(Let &let) noexcept {
    if (let.attributes().isPublic()) {
      out << "public ";
    } else {
      out << "private ";
    }

    out << "let " << let.name();

    if (auto annotation = let.annotation()) {
      out << ": " << annotation.value();
    }

    out << " = ";
    print(out, mir, let.parameter());
    out << ";";
  }

  void operator()(Binop &binop) noexcept {
    print(out, mir, binop.left());
    out << " " << binop.op() << " ";
    print(out, mir, binop.right());
  }

  void operator()(Unop &unop) noexcept {
    out << " " << unop.op();
    print(out, mir, unop.right());
  }

  void operator()(Import &i) noexcept { out << "import " << i.file() << ";"; }

  void operator()(Module &m) noexcept {
    out << "module " << m.name() << " { \n";

    for (auto &expression : m.expressions()) {
      print(out, expression);
      out << "\n";
    }

    out << "}";
  }

  void operator()(Call &call) noexcept {
    print(out, mir, call.callee());
    out << "(";

    auto index = 0U;
    auto size = call.arguments().size();
    for (auto &argument : call.arguments()) {
      print(out, mir, argument);

      if (index++ < (size - 1)) {
        out << ", ";
      }
    }

    out << ")";
  }

  void operator()(Lambda &lambda) noexcept {
    out << "\\";

    auto index = 0U;
    auto size = lambda.arguments().size();
    for (auto argument : lambda.arguments()) {
      out << argument.name << ": " << argument.type;

      if (index++ < (size - 1)) {
        out << ", ";
      }
    }

    auto annotation = lambda.annotation();
    if (annotation) {
      out << " -> " << annotation.value();
    }

    out << " => ";
    print(out, lambda.body());
  }
};

void print(std::ostream &out, Mir &mir, detail::Index index) noexcept {
  PrintInstructionVisitor visitor(out, mir);
  visitor(index);
}

struct PrintValueVisitor {
  std::ostream &out;
  PrintValueVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(ir::Value &value) noexcept {
    std::visit(*this, value.variant());
  }

  void operator()(ir::Scalar &scalar) noexcept { print(out, scalar); }

  void operator()(ir::Lambda &lambda) noexcept {
    out << "\\";

    auto index = 0U;
    auto size = lambda.arguments().size();
    for (auto argument : lambda.arguments()) {
      out << argument.name << ": " << argument.type;

      if (index++ < (size - 1)) {
        out << ", ";
      }
    }

    auto annotation = lambda.annotation();
    if (annotation) {
      out << " -> " << annotation.value();
    }

    out << " => ";
    print(out, lambda.body());
  }
};

void print(std::ostream &out, ir::Value &value) noexcept {
  PrintValueVisitor visitor(out);
  visitor(value);
}

void print(std::ostream &out, Mir &mir) noexcept {
  print(out, mir, mir.root());
}
} // namespace mint::ir