cmake_minimum_required(VERSION 3.20)

set(MINT_IR_SOURCES 
  ${MINT_SOURCE_DIR}/ir/statement/Module.cpp 
  ${MINT_SOURCE_DIR}/ir/Mir.cpp 
)

add_library(mint_ir STATIC 
  ${MINT_IR_SOURCES}
)
target_include_directories(mint_ir PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_ir PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_ir PUBLIC cxx_std_23)
target_link_options(mint_ir PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_scan
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)