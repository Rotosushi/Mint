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
#include <string_view>
#include <vector>

#include "utility/Config.hpp"

namespace mint {
class OptionsParser {
private:
  // #TODO: implement behavior for Optional, Required, and Positional.
  enum ArgumentStyle { None, Optional, Required, Positional };

  // #NOTE:
  // the text which is matched when looking for the option.
  // the tag returned by the argument matcher
  // a description of the option for help
  // if the option takes an argument ParseArgument returns the argument
  struct Option {
    std::string_view text;
    char tag;
    std::string_view description;
    ArgumentStyle argument_style;
  };

  std::vector<Option> options = {
      {"h", 'h', "print valid program argument usage", None},
      {"help", 'h', "print valid program argument usage", None},
      {"v", 'v', "print version information and exit", None},
      {"version", 'v', "print version information and exit", None},
      {"compile", 'c', "compile the given input file", Required}};

  std::vector<std::string_view> arguments;

  char parseArgument(std::string_view argument) {
    if (argument.empty())
      return '?';

    // an option starts with '-'
    if (argument[0] != '-')
      return '~'; // argument is not an option

    argument = argument.substr(1);
    // an option can start with "-" or "--"
    if (argument[0] == '-')
      argument = argument.substr(1);

    for (const auto &option : options) {
      if (argument[0] != option.text[0])
        continue;

      if (option.text == argument) {
        return option.tag;
      }
    }
    // unknown option
    return '?';
  }

  void printHelp(std::ostream &out) {
    out << "mint usage: \n\t\t mint [option]\n\n";
    for (auto &option : options) {
      out << "-- or -" << option.text << " : " << option.description << "\n";
    }
    out << "\n";
  }

  void printVersion(std::ostream &out) noexcept {
    out << "mint version " << MINT_VERSION_MAJOR << "." << MINT_VERSION_MINOR
        << "." << MINT_VERSION_PATCH << "\n git revision [" << MINT_GIT_REVISION
        << "]\n Compiled on " << __DATE__ << " at " << __TIME__ << "\n";
  }

  void printUnknownOption(std::ostream &out, std::string_view option) {
    out << "unknown option: " << option << "\n";
  }

public:
  OptionsParser(int argc, char **argv) noexcept {
    // start from 1, because argv[0] is defined to be
    // the program name (or an empty string)
    for (int index = 1; index < argc; ++index) {
      arguments.emplace_back(argv[index]);
    }
  }

  auto parse() noexcept {
    for (auto &argument : arguments) {
      switch (parseArgument(argument)) {
      case 'h': {
        printHelp(std::cout);
        std::exit(EXIT_SUCCESS);
        break;
      }

      case 'v': {
        printVersion(std::cout);
        std::exit(EXIT_SUCCESS);
        break;
      }

      case '?': {
        printUnknownOption(std::cerr, argument);
        break;
      }

      case '~': // not an option
        continue;
      }
    }
  }
};
} // namespace mint
