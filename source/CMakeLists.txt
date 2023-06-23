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
  ${MINT_INCLUDE_DIR}/adt/Attributes.hpp
  ${MINT_INCLUDE_DIR}/adt/BinopTable.hpp
  ${MINT_SOURCE_DIR}/adt/BinopTable.cpp
  ${MINT_INCLUDE_DIR}/adt/Environment.hpp
  ${MINT_SOURCE_DIR}/adt/Environment.cpp
  ${MINT_INCLUDE_DIR}/adt/DirectorySearch.hpp 
  ${MINT_INCLUDE_DIR}/adt/Identifier.hpp 
  ${MINT_SOURCE_DIR}/adt/Identifier.cpp 
  ${MINT_INCLUDE_DIR}/adt/Scope.hpp
  ${MINT_SOURCE_DIR}/adt/Scope.cpp
  ${MINT_INCLUDE_DIR}/adt/TypeInterner.hpp 
  ${MINT_INCLUDE_DIR}/adt/UnopTable.hpp
  ${MINT_SOURCE_DIR}/adt/UnopTable.cpp

  ${MINT_INCLUDE_DIR}/ast/Ast.hpp 
  ${MINT_INCLUDE_DIR}/ast/Clone.hpp
  ${MINT_INCLUDE_DIR}/ast/Equals.hpp
  ${MINT_INCLUDE_DIR}/ast/Evaluate.hpp
  ${MINT_INCLUDE_DIR}/ast/Print.hpp
  ${MINT_INCLUDE_DIR}/ast/Typecheck.hpp

  ${MINT_INCLUDE_DIR}/error/Error.hpp
  ${MINT_SOURCE_DIR}/error/Error.cpp
  ${MINT_INCLUDE_DIR}/error/Result.hpp

  ${MINT_INCLUDE_DIR}/scan/Token.hpp
  ${MINT_INCLUDE_DIR}/scan/Location.hpp
  ${MINT_INCLUDE_DIR}/scan/Parser.hpp
  ${MINT_SOURCE_DIR}/scan/Parser.cpp
  ${MINT_INCLUDE_DIR}/scan/Scanner.hpp
  ${MINT_SOURCE_DIR}/scan/Scanner.cpp

  ${MINT_INCLUDE_DIR}/type/Equals.hpp
  ${MINT_INCLUDE_DIR}/type/Print.hpp
  ${MINT_INCLUDE_DIR}/type/Type.hpp 

  ${MINT_INCLUDE_DIR}/utility/Abort.hpp
  ${MINT_INCLUDE_DIR}/utility/Allocator.hpp 
  ${MINT_INCLUDE_DIR}/utility/Assert.hpp
  ${MINT_INCLUDE_DIR}/utility/Config.hpp
  ${MINT_INCLUDE_DIR}/utility/Log.hpp 
  ${MINT_INCLUDE_DIR}/utility/NumbersRoundTrip.hpp
  ${MINT_INCLUDE_DIR}/utility/OptionsParser.hpp
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
