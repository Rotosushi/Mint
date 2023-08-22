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
#include <iostream>
#include <list>
#include <stack>
#include <vector>

#include "adt/SourceBuffer.hpp"

namespace mint {
class SourceBufferList {
public:
  // we need access to the SourceBuffers in a stack like manner
  // and we want SourceBuffers to be alive after they are not
  // active, such that SourceLocation can maintain a string_view
  // into it's corresponding SourceBuffer.

  using List = std::list<SourceBuffer>;
  using Stack = std::stack<SourceBuffer *, std::vector<SourceBuffer *>>;

private:
  List m_list;
  Stack m_stack;

public:
  SourceBufferList() {
    auto buffer = &m_list.emplace_back(InputStream{&std::cin});
    m_stack.push(buffer);
  }

  SourceBuffer *peek() { return m_stack.top(); }
  SourceBuffer *push(std::fstream &&fin) {
    auto buffer = &m_list.emplace_back(InputStream{std::move(fin)});
    m_stack.push(buffer);
    return buffer;
  }
  SourceBuffer *pop() {
    auto top = m_stack.top();
    m_stack.pop();
    return top;
  }
};
} // namespace mint
