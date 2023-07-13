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
  main.cpp
)
target_link_libraries(mint_test
  mint_common
  ${Boost_LIBRARIES}
)