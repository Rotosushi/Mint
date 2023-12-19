cmake_minimum_required(VERSION 3.20)


#add_subdirectory(${MINT_SOURCE_DIR}/ir)
#add_subdirectory(${MINT_SOURCE_DIR}/scan)
#add_subdirectory(${MINT_SOURCE_DIR}/type)
#add_subdirectory(${MINT_SOURCE_DIR}/adt)
#add_subdirectory(${MINT_SOURCE_DIR}/runtime)
#add_subdirectory(${MINT_SOURCE_DIR}/comptime)


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
  ${MINT_SOURCE_DIR}/scan/Lexer.re 
  ${MINT_SOURCE_DIR}/scan/Lexer.cpp
)

add_library(mint_common STATIC 
  ${MINT_SOURCE_DIR}/adt/Attributes.cpp 
  ${MINT_SOURCE_DIR}/adt/BinopTable.cpp 
  ${MINT_SOURCE_DIR}/adt/DirectorySearch.cpp 
  ${MINT_SOURCE_DIR}/adt/Environment.cpp 
  ${MINT_SOURCE_DIR}/adt/Error.cpp 
  ${MINT_SOURCE_DIR}/adt/Identifier.cpp 
  ${MINT_SOURCE_DIR}/adt/Recoverable.cpp 
  ${MINT_SOURCE_DIR}/adt/SourceBuffer.cpp 
  ${MINT_SOURCE_DIR}/adt/Scope.cpp 
  ${MINT_SOURCE_DIR}/adt/UnopTable.cpp 
  ${MINT_SOURCE_DIR}/adt/UseBeforeDefMap.cpp 

  ${MINT_SOURCE_DIR}/comptime/Repl.cpp 
  ${MINT_SOURCE_DIR}/comptime/Compile.cpp 
  ${MINT_SOURCE_DIR}/comptime/Emit.cpp 
  ${MINT_SOURCE_DIR}/comptime/Import.cpp 
  ${MINT_SOURCE_DIR}/comptime/Parse.cpp 
  ${MINT_SOURCE_DIR}/comptime/Typecheck.cpp 
  ${MINT_SOURCE_DIR}/comptime/Evaluate.cpp 
  ${MINT_SOURCE_DIR}/comptime/Codegen.cpp 
  ${MINT_SOURCE_DIR}/comptime/Link.cpp

  ${MINT_SOURCE_DIR}/runtime/Allocate.cpp 
  ${MINT_SOURCE_DIR}/runtime/ForwardDeclare.cpp 
  ${MINT_SOURCE_DIR}/runtime/Load.cpp 
  ${MINT_SOURCE_DIR}/runtime/Store.cpp 

  ${MINT_SOURCE_DIR}/ast/visitors/HasSideEffect.cpp 
  ${MINT_SOURCE_DIR}/ast/visitors/Print.cpp

#  ${MINT_SOURCE_DIR}/ir/action/Print.cpp 
#  ${MINT_SOURCE_DIR}/ir/action/Clone.cpp 
#  ${MINT_SOURCE_DIR}/ir/definition/Function.cpp
#  ${MINT_SOURCE_DIR}/ir/questions/IsDefinition.cpp 
#  ${MINT_SOURCE_DIR}/ir/statement/Module.cpp 
#  ${MINT_SOURCE_DIR}/ir/Mir.cpp 

  ${MINT_SOURCE_DIR}/scan/Parser.cpp 
  ${MINT_SOURCE_DIR}/scan/Lexer.cpp 
  ${MINT_SOURCE_DIR}/scan/Token.cpp 

  ${MINT_SOURCE_DIR}/type/Type.cpp 
)
set_property(
  TARGET mint_common 
  APPEND 
  PROPERTY ADDITIONAL_CLEAN_FILES ${MINT_SOURCE_DIR}/scan/Lexer.cpp 
)
target_include_directories(mint_common PUBLIC ${MINT_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS})
target_compile_options(mint_common PUBLIC ${CXX_OPTIONS} ${llvm_cxxflags})
target_compile_features(mint_common PUBLIC cxx_std_23)
target_link_options(mint_common PUBLIC 
  #-v
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
)
