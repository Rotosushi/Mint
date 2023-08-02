cmake_minimum_required(VERSION 3.20)

set(WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
set(CXX_OPTIONS ${WARNINGS})

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

add_library(mint_common STATIC 
  ${MINT_SOURCE_DIR}/adt/Attributes.cpp 
  ${MINT_SOURCE_DIR}/adt/BinopTable.cpp
  ${MINT_SOURCE_DIR}/adt/DirectorySearch.cpp 
  ${MINT_SOURCE_DIR}/adt/Environment.cpp
  ${MINT_SOURCE_DIR}/adt/Identifier.cpp 
  ${MINT_SOURCE_DIR}/adt/ImportSet.cpp
  ${MINT_SOURCE_DIR}/adt/Scope.cpp
  ${MINT_SOURCE_DIR}/adt/UnopTable.cpp
  ${MINT_SOURCE_DIR}/adt/UseBeforeDefMap.cpp 

  ${MINT_SOURCE_DIR}/ast/definition/Function.cpp 
  ${MINT_SOURCE_DIR}/ast/definition/Let.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Binop.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Unop.cpp
  ${MINT_SOURCE_DIR}/ast/expression/Variable.cpp
  ${MINT_SOURCE_DIR}/ast/statement/Import.cpp
  ${MINT_SOURCE_DIR}/ast/statement/Module.cpp 
  ${MINT_SOURCE_DIR}/ast/syntax/Affix.cpp 
  ${MINT_SOURCE_DIR}/ast/syntax/Parens.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Nil.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Boolean.cpp 
  ${MINT_SOURCE_DIR}/ast/value/Integer.cpp 

  ${MINT_SOURCE_DIR}/error/Error.cpp

  ${MINT_SOURCE_DIR}/scan/Parser.cpp
  ${MINT_SOURCE_DIR}/scan/Scanner.cpp
  ${MINT_SOURCE_DIR}/scan/Token.cpp

  ${MINT_SOURCE_DIR}/type/Nil.cpp
  ${MINT_SOURCE_DIR}/type/Boolean.cpp
  ${MINT_SOURCE_DIR}/type/Integer.cpp

  ${MINT_SOURCE_DIR}/utility/Abort.cpp
  ${MINT_SOURCE_DIR}/utility/CommandLineOptions.cpp
  ${MINT_SOURCE_DIR}/utility/LLVMUtility.cpp
  ${MINT_SOURCE_DIR}/utility/Log.cpp
)
set_property(
  TARGET mint_common 
  APPEND 
  PROPERTY ADDITIONAL_CLEAN_FILES ${MINT_SOURCE_DIR}/scan/Scanner.cpp 
)
target_include_directories(mint_common PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_common PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_common PUBLIC cxx_std_23)
target_link_options(mint_common PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint_common
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)


add_executable(mint main.cpp)
target_include_directories(mint PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint PUBLIC cxx_std_23)
target_link_options(mint PUBLIC 
  ${llvm_ldflags} 
  ${llvm_system_libs}
)
target_link_libraries(mint 
  mint_common
  ${llvm_libs}
  ${LLVM_LIBRARY_DIRS}/liblldCommon.a
  ${LLVM_LIBRARY_DIRS}/liblldELF.a
)
