cmake_minimum_required(VERSION 3.20)

add_library(mint_comptime STATIC
  ${MINT_SOURCE_DIR}/comptime/Repl.cpp 
  ${MINT_SOURCE_DIR}/comptime/Compile.cpp
  ${MINT_SOURCE_DIR}/comptime/Emit.cpp 
  ${MINT_SOURCE_DIR}/comptime/Import.cpp
  ${MINT_SOURCE_DIR}/comptime/Parse.cpp
  ${MINT_SOURCE_DIR}/comptime/Typecheck.cpp 
  ${MINT_SOURCE_DIR}/comptime/Evaluate.cpp 
  ${MINT_SOURCE_DIR}/comptime/Codegen.cpp
)
target_include_directories(mint_comptime PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_comptime PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_comptime PUBLIC cxx_std_23)
target_link_options(mint_comptime PUBLIC 
  ${llvm_ldflags}
  ${llvm_system_libs}
)
target_link_libraries(mint_comptime 
  mint_ir
  mint_adt
  mint_runtime
  ${llvm_libs}
)