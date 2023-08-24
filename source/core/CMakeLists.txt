cmake_minimum_required(VERSION 3.20)

add_library(mint_core STATIC
  ${MINT_SOURCE_DIR}/core/Repl.cpp 
  ${MINT_SOURCE_DIR}/core/Typecheck.cpp 
  ${MINT_SOURCE_DIR}/core/Evaluate.cpp 
  ${MINT_SOURCE_DIR}/core/Codegen.cpp
)
target_include_directories(mint_core PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_core PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_core PUBLIC cxx_std_23)
target_link_options(mint_core PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_core 
  mint_ir 
  mint_adt
)