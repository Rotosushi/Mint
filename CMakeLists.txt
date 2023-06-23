cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mint LANGUAGES C CXX VERSION 0.0.1 DESCRIPTION "A minimal interpreter")

message(STATUS "c++ standard: ${CMAKE_CXX_STANDARD}")

set(MINT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(MINT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")



find_package(LLVM 16 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# link against all llvm libraries available.
execute_process(
	COMMAND llvm-config --libs 
	OUTPUT_VARIABLE llvm_libs 
	OUTPUT_STRIP_TRAILING_WHITESPACE 
)

separate_arguments(llvm_libs)

# retrieved from llvm-config --cxxflags
add_compile_options(
  -I/usr/local/include 
  -fno-exceptions
  -fno-rtti 
  -D_GNU_SOURCE
  -D_DEBUG 
  -D__STDC_CONSTANT_MACROS
  -D__STDC_FORMAT_MACROS
  -D__STDC_LIMIT_MACROS
)

execute_process(
	COMMAND llvm-config --ldflags
	OUTPUT_VARIABLE llvm_ldflags
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(llvm_ldflags)

execute_process(
  COMMAND llvm-config --system-libs
  OUTPUT_VARIABLE llvm_syslibs
  OUTPUT_STRIP_TRAILING_WHITESPACE  
)

separate_arguments(llvm_syslibs)



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
