cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mint VERSION 0.0.1 DESCRIPTION "A minimal interpreter")

message(STATUS "c++ standard: ${CMAKE_CXX_STANDARD}")

set(MINT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(MINT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")

#project wide configuration information
execute_process(
  COMMAND git rev-parse HEAD
  OUTPUT_VARIABLE Mint_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${MINT_INCLUDE_DIR}/utility/Config.hpp.in 
  ${MINT_INCLUDE_DIR}/utility/Config.hpp
)

# the subdirectory contains the rules to build to executable and tests
add_subdirectory(source)
