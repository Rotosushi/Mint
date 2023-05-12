cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mint VERSION 0.0.1 DESCRIPTION "A minimal interpreter")

#project wide configuration information
execute_process(
  COMMAND git rev-parse HEAD
  OUTPUT_VARIABLE Mint_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${PROJECT_SOURCE_DIR}/include/utility/Config.hpp.in 
  ${PROJECT_SOURCE_DIR}/include/utility/Config.hpp
)

# the subdirectory contains the rules to build to executable and tests
add_subdirectory(source)
