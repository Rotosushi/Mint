cmake_minimum_required(VERSION 3.20)

message(STATUS "cmake c++ flags: ${CMAKE_CXX_FLAGS}")

add_executable(mint main.cpp)
target_include_directories(mint PUBLIC ../include)
