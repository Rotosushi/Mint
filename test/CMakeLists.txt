cmake_minimum_required(VERSION 3.20)

find_package(Boost 
COMPONENTS unit_test_framework
REQUIRED 
)

message(STATUS "Found Boost ${Boost_VERSION} in ${Boost_INCLUDE_DIRS}")


set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(TEST_INCLUDE_DIR ${TEST_DIR}/include)
set(TEST_SOURCE_DIR ${TEST_DIR}/source)
set(Mint_TEST_RESOURCES_DIR ${TEST_DIR}/resources)

configure_file(
  ${TEST_INCLUDE_DIR}/Config.hpp.in 
  ${TEST_INCLUDE_DIR}/Config.hpp
)

add_executable(mint_test
  ${TEST_INCLUDE_DIR}/TestExpressions.hpp

  ${TEST_SOURCE_DIR}/REPL.cpp 

  main.cpp
)
target_include_directories(mint_test PUBLIC
 ${TEST_INCLUDE_DIR}
)
target_link_libraries(mint_test
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  mint_common
)