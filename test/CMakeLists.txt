cmake_minimum_required(VERSION 3.20)

find_package(Catch2 3 REQUIRED)

set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(TEST_INCLUDE_DIR ${TEST_DIR}/include)
set(TEST_SOURCE_DIR ${TEST_DIR}/source)

add_executable(mint_test
  ${TEST_SOURCE_DIR}/CompiledCode.cpp

  ${TEST_SOURCE_DIR}/REPL.cpp 

  main.cpp
)
target_include_directories(mint_test PUBLIC
 ${TEST_INCLUDE_DIR}
 ${MINT_INCLUDE_DIR}
)
target_link_libraries(mint_test
  mint_common
  Catch2::Catch2
)

include(CTest)
include(Catch)
catch_discover_tests(mint_test)