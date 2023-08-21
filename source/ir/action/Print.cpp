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

namespace mint::ir {
struct PrintScalarVisitor {
  std::ostream &out;
  PrintScalarVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(detail::Scalar &scalar) noexcept {
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

void print(std::ostream &out, detail::Scalar &scalar) {
  PrintScalarVisitor visitor(out);
  visitor(scalar);
}

struct PrintImmediateVisitor {
  std::ostream &out;
  PrintImmediateVisitor(std::ostream &out) noexcept : out(out) {}

  void operator()(detail::Immediate &immediate) noexcept {
    std::visit(*this, immediate.variant());
  }

  void operator()(detail::Scalar &scalar) noexcept { print(out, scalar); }

  void operator()(Identifier &name) noexcept { out << name; }
};

void print(std::ostream &out, detail::Immediate &immediate) noexcept {
  PrintImmediateVisitor visitor(out);
  visitor(out, immediate);
}

struct print(std::ostream &out, Mir &mir, detail::Index index) noexcept;

struct PrintParameterVisitor {
  std::ostream &out;
  Mir &mir;
  PrintParameterVisitor(std::ostream &out, Mir &mir) noexcept
      : out(out), mir(mir) {}

  void operator()(detail::Parameter &parameter) noexcept {
    std::visit(*this, parameter.variant());
  }

  void operator()(detail::Immediate &immediate) noexcept {
    print(out, immediate);
  }

  void operator()(detail::Index &index) noexcept { print(out, mir, index); }
};

void print(std::ostream &out, Mir &mir, detail::Parameter &parameter) noexcept {
  PrintParameterVisitor visitor(out, mir);
  visitor(parameter);
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

  void operator()(Affix &affix) noexcept {
    print(out, mir, affix.parameter());
    out << ";";
  }

  void operator()(Parens &parens) noexcept {
    out << "(";
    print(out, mir, parens.parameter());
    out << ")";
  }

  void operator()(Let &let) noexcept {
    out << "let " << let.name();
    out << " = ";
    print(out, mir, let.parameter());
  }
};

void print(std::ostream &out, Mir &mir) noexcept {
  print(out, mir, mir.root());
}
} // namespace mint::ir