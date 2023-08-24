cmake_minimum_required(VERSION 3.20)

add_library(mint_type STATIC 
  ${MINT_SOURCE_DIR}/type/Type.cpp
)
target_include_directories(mint_type PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_type PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_type PUBLIC cxx_std_23)
target_link_options(mint_type PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_type
  #${llvm_libs}
  #${LLVM_LIBRARY_DIRS}/liblldCommon.a
  #{LLVM_LIBRARY_DIRS}/liblldELF.a
)