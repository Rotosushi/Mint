cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mint LANGUAGES C CXX VERSION 0.0.3 DESCRIPTION "A minimal interpreter")

message(STATUS "c++ standard: ${CMAKE_CXX_STANDARD}")

set(MINT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(MINT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")
set(MINT_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/resources")

set(WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
#set(CXX_OPTIONS ${WARNINGS})
set(CXX_OPTIONS ${WARNINGS} -flto=auto -fuse-linker-plugin)

# find dependencies
# find LLVM
find_package(LLVM 16 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# retrieved from llvm-config --cxxflags 
# NOTE: explicitly listed to remove -std=c++20 flag
# NOTE: we cannot add -fno-exceptions and use boost test.
set(llvm_cxxflags 
  -I/usr/local/include 
  #-fno-exceptions 
  -fno-rtti 
  -D_GNU_SOURCE 
  -D_DEBUG 
  -D__STDC_CONSTANT_MACROS 
  -D__STDC_FORMAT_MACROS 
  -D__STDC_LIMIT_MACROS
)

execute_process(
  COMMAND llvm-config --libfiles
  OUTPUT_VARIABLE llvm_libs 
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(llvm_libs)


execute_process(
  COMMAND llvm-config --ldflags 
  OUTPUT_VARIABLE llvm_ldflags 
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(llvm_ldflags)

execute_process(
  COMMAND llvm-config --system-libs 
  OUTPUT_VARIABLE llvm_system_libs
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(llvm_system_libs)


# create project wide configuration information
execute_process(
  COMMAND git rev-parse HEAD
  OUTPUT_VARIABLE Mint_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ($<CONFIG:Debug> OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(MINT_DEBUG 1)
else()
  set(MINT_DEBUG 0)
endif()

configure_file(
  ${MINT_INCLUDE_DIR}/utility/Config.hpp.in 
  ${MINT_INCLUDE_DIR}/utility/Config.hpp
)

# build the mint executable
add_subdirectory(source)

# build the mint_test executable
if (MINT_BUILD_TESTS)
  add_subdirectory(test)
endif()
