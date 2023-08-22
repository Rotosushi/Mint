cmake_minimum_required(VERSION 3.20)

add_library(mint_adt STATIC 
  ${MINT_SOURCE_DIR}/adt/Attributes.cpp 
  ${MINT_SOURCE_DIR}/adt/BinopTable.cpp
  ${MINT_SOURCE_DIR}/adt/DirectorySearch.cpp 
  ${MINT_SOURCE_DIR}/adt/Environment.cpp
  ${MINT_SOURCE_DIR}/adt/Error.cpp 
  ${MINT_SOURCE_DIR}/adt/Identifier.cpp 
  ${MINT_SOURCE_DIR}/adt/ImportSet.cpp
  ${MINT_SOURCE_DIR}/adt/Recoverable.cpp

  ${MINT_SOURCE_DIR}/adt/SourceBuffer.cpp
  ${MINT_SOURCE_DIR}/adt/Scope.cpp
  ${MINT_SOURCE_DIR}/adt/UnopTable.cpp
  ${MINT_SOURCE_DIR}/adt/UseBeforeDefMap.cpp 
)
target_include_directories(mint_adt PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_adt PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_adt PUBLIC cxx_std_23)
target_link_options(mint_adt PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_adt
  mint_ast
  mint_type
  mint_scan
  mint_codegen
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)