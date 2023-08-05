cmake_minimum_required(VERSION 3.20)

add_library(mint_ast STATIC 
  ${MINT_SOURCE_DIR}/ast/definition/Function.cpp 
  ${MINT_SOURCE_DIR}/ast/definition/Let.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Binop.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Call.cpp 
  ${MINT_SOURCE_DIR}/ast/expression/Unop.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Variable.cpp
  ${MINT_SOURCE_DIR}/ast/statement/Import.cpp
  ${MINT_SOURCE_DIR}/ast/statement/Module.cpp 
  ${MINT_SOURCE_DIR}/ast/syntax/Affix.cpp 
  ${MINT_SOURCE_DIR}/ast/syntax/Parens.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Nil.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Boolean.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Integer.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Lambda.cpp
)
target_include_directories(mint_ast PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_ast PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_ast PUBLIC cxx_std_23)
target_link_options(mint_ast PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_ast
  mint_codegen
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)