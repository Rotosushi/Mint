cmake_minimum_required(VERSION 3.20)

function(re2c_generate_scanner input output)
add_custom_command(
  OUTPUT ${output}
  COMMAND re2c ${input} -o ${output}
  DEPENDS ${input}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Generating ${output}"
)
endfunction()

re2c_generate_scanner(
  ${MINT_SOURCE_DIR}/scan/Scanner.re
  ${MINT_SOURCE_DIR}/scan/Scanner.cpp
)

add_library(mint_scan STATIC 
  ${MINT_SOURCE_DIR}/scan/Parser.cpp
  ${MINT_SOURCE_DIR}/scan/Scanner.cpp
  ${MINT_SOURCE_DIR}/scan/Token.cpp
)
set_property(
  TARGET mint_scan 
  APPEND 
  PROPERTY ADDITIONAL_CLEAN_FILES ${MINT_SOURCE_DIR}/scan/Scanner.cpp 
)
target_include_directories(mint_scan PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_scan PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_scan PUBLIC cxx_std_23)
target_link_options(mint_scan PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_scan
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)