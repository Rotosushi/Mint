cmake_minimum_required(VERSION 3.20)

message(STATUS "cmake c++ flags: ${CMAKE_CXX_FLAGS}")

set(WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
set(CXX_OPTIONS ${WARNINGS})

#add_library(mint_common STATIC 
#)

#target_compile_options(mint_common PUBLIC ${CXX_OPTIONS})
#target_include_directories(mint_common PUBLIC ../include)

add_executable(mint main.cpp)
target_include_directories(mint PUBLIC ../include)
target_compile_options(mint PUBLIC ${CXX_OPTIONS})
#target_link_libraries(mint mint_common)
