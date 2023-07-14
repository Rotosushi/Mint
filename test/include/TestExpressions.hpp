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
#include <string_view>
#include <vector>

struct TestExpression {
  std::string_view test_code;
  std::string_view expected_result;
};

std::vector<TestExpression> getTestExpressions() noexcept {
  std::vector<TestExpression> expressions{
      {"nil;", "nil"}, {"1;", "1"}, {"true;", "true"}, {"false;", "false"}};

  return expressions;
}