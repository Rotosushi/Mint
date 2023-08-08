cmake_minimum_required(VERSION 3.20)

find_package(Boost 
COMPONENTS unit_test_framework
REQUIRED 
)

message(STATUS "Found Boost ${Boost_VERSION} in ${Boost_INCLUDE_DIRS}")

set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(TEST_INCLUDE_DIR ${TEST_DIR}/include)
set(TEST_SOURCE_DIR ${TEST_DIR}/source)

add_executable(mint_test
  ${TEST_SOURCE_DIR}/REPL.cpp 

  main.cpp
)
target_include_directories(mint_test PUBLIC
 ${TEST_INCLUDE_DIR}
 ${MINT_INCLUDE_DIR}
)
target_link_libraries(mint_test
  mint_scan
  mint_type
  mint_ast
  mint_adt
  mint_codegen
  mint_utility
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)