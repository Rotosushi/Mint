cmake_minimum_required(VERSION 3.20)

add_library(mint_utility STATIC 
  ${MINT_SOURCE_DIR}/utility/Abort.cpp
  ${MINT_SOURCE_DIR}/utility/CommandLineOptions.cpp
  ${MINT_SOURCE_DIR}/utility/Log.cpp
)
target_include_directories(mint_utility PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_utility PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_utility PUBLIC cxx_std_23)
target_link_options(mint_utility PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_utility
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)