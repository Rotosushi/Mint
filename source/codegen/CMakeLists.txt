cmake_minimum_required(VERSION 3.20)

add_library(mint_codegen STATIC 
  ${MINT_SOURCE_DIR}/codegen/LLVMUtility.cpp 
  ${MINT_SOURCE_DIR}/codegen/Allocate.cpp 
  ${MINT_SOURCE_DIR}/codegen/Load.cpp
  ${MINT_SOURCE_DIR}/codegen/Store.cpp
)
target_include_directories(mint_codegen PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_codegen PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_codegen PUBLIC cxx_std_23)
target_link_options(mint_codegen PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_codegen
  mint_adt
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)