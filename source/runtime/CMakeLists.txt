cmake_minimum_required(VERSION 3.20)

add_library(mint_runtime STATIC 
  ${MINT_SOURCE_DIR}/runtime/Allocate.cpp 
  ${MINT_SOURCE_DIR}/runtime/ForwardDeclare.cpp
  
  ${MINT_SOURCE_DIR}/runtime/Load.cpp
  ${MINT_SOURCE_DIR}/runtime/Store.cpp
)
target_include_directories(mint_runtime PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_runtime PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_runtime PUBLIC cxx_std_23)
target_link_options(mint_runtime PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_runtime
  mint_adt
  ${llvm_libs}
  #${LLVM_LIBRARY_DIRS}/liblldCommon.a
  #${LLVM_LIBRARY_DIRS}/liblldELF.a
)