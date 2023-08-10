cmake_minimum_required(VERSION 3.20)

add_library(mint_typecheck STATIC
  ${MINT_SOURCE_DIR}/typecheck/Typecheck.cpp
)
target_include_directories(mint_typecheck PUBLIC ${MINT_INCLUDE_DIR})
target_compile_options(mint_typecheck PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_typecheck PUBLIC cxx_std_23)
target_link_options(mint_typecheck PUBLIC
  ${llvm_ldflags}
)
target_link_libraries(mint_typecheck
  mint_adt
  mint_ir
)