cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mint VERSION 0.0.1 DESCRIPTION "A minimal interpreter")

configure_file(
  ${PROJECT_SOURCE_DIR}/include/utility/Config.hpp.in 
  ${PROJECT_SOURCE_DIR}/include/utility/Config.hpp
)

add_subdirectory(source)
