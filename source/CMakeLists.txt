cmake_minimum_required(VERSION 3.20)

set(MINT_COMMON_LIBS
  mint_type
  mint_adt
  mint_core
  mint_utility
  mint_ir
  mint_codegen
  mint_scan
)

add_subdirectory(${MINT_SOURCE_DIR}/adt)
add_subdirectory(${MINT_SOURCE_DIR}/scan)
add_subdirectory(${MINT_SOURCE_DIR}/type)
add_subdirectory(${MINT_SOURCE_DIR}/utility)
add_subdirectory(${MINT_SOURCE_DIR}/codegen)
add_subdirectory(${MINT_SOURCE_DIR}/ir)
add_subdirectory(${MINT_SOURCE_DIR}/core)

add_executable(mint main.cpp)
target_include_directories(mint PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint PUBLIC cxx_std_23)
target_link_options(mint PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint 
  ${MINT_COMMON_LIBS}
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)
