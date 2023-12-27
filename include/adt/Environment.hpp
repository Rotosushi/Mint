// Copyright (C) 2023 Cade Weinberg
//
// This file is part of Mint.
//
// Mint is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Mint is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Mint.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <iostream>

#include "llvm/ADT/StringSet.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"

#include "adt/BinopTable.hpp"
#include "adt/DirectorySearch.hpp"
#include "adt/Identifier.hpp"
#include "adt/ImportSet.hpp"
#include "adt/InsertionPoint.hpp"
#include "adt/Scope.hpp"
#include "adt/TranslationUnit.hpp"
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"
#include "adt/UseBeforeDefMap.hpp"
#include "scan/Parser.hpp"

namespace mint {
// Allocates the data-structures necessary for the
// compilation process.
class Environment {
  std::istream *m_input;
  std::ostream *m_output;
  std::ostream *m_error_output;
  std::ostream *m_log_output;

  std::optional<fs::path> m_file;
  std::shared_ptr<Scope> m_global_scope;
  std::shared_ptr<Scope> m_local_scope;

  TranslationUnit m_translation_unit;
  DirectorySearcher m_directory_searcher;
  ImportSet m_imported_files;
  TypeInterner m_type_interner;
  StringSet m_string_set;
  BinopTable m_binop_table;
  UnopTable m_unop_table;
  UseBeforeDefMap m_use_before_def_map;

  // Parser m_parser;
  Parser m_parser;

  std::unique_ptr<llvm::LLVMContext> m_llvm_context;
  std::unique_ptr<llvm::Module> m_llvm_module;
  std::unique_ptr<llvm::IRBuilder<>> m_llvm_ir_builder;
  llvm::TargetMachine *m_llvm_target_machine;

  Environment(std::istream *in, std::ostream *out, std::ostream *errout,
              std::ostream *log,
              std::unique_ptr<llvm::LLVMContext> llvm_context,
              std::unique_ptr<llvm::Module> llvm_module,
              std::unique_ptr<llvm::IRBuilder<>> llvm_ir_builder,
              llvm::TargetMachine *llvm_target_machine) noexcept;

public:
  /**** Environment Methods ****/
  [[nodiscard]] static auto nativeCPUFeatures() noexcept -> std::string;
  //   #NOTE: does the work of initializing the llvm data structures
  [[nodiscard]] static auto create(std::istream *in = &std::cin,
                                   std::ostream *out = &std::cout,
                                   std::ostream *errout = &std::cerr,
                                   std::ostream *log = &std::clog) noexcept
      -> Environment;

  //**** Environment member interfaces ****//
  std::istream &inputStream() noexcept;
  std::ostream &outputStream() noexcept;
  std::ostream &errorStream() noexcept;
  std::ostream &logStream() noexcept;

  std::optional<fs::path> const &sourceFile() noexcept;
  void sourceFile(fs::path file) noexcept;

  llvm::TargetMachine &targetMachine() noexcept {
    return *m_llvm_target_machine;
  }

  //**** Parser interface ****//
  auto endOfMirInput() const noexcept -> bool;
  Result<ast::Ptr> parse();

  void pushActiveSourceFile(std::fstream &&fin);
  void popActiveSourceFile();

  //**** TranslationUnit interface ****//
  void addLocalExpression(ast::Ptr p) noexcept {
    m_translation_unit.append(std::move(p));
  }
  auto localExpressions() noexcept -> TranslationUnit::Expressions & {
    return m_translation_unit.expressions;
  }

  //**** DirectorySearcher interface ****//
  void appendDirectories(std::vector<std::string> const &paths) noexcept;
  void appendDirectory(fs::path const &file) noexcept;
  auto fileExists(fs::path const &file) noexcept -> bool;
  auto fileSearch(fs::path const &file) noexcept -> std::optional<std::fstream>;

  //**** Identifier Set Interface ****//
  auto getIdentifier(std::string_view name) noexcept -> Identifier;
  auto getLambdaName() noexcept -> Identifier;

  //**** ImportSet interface ****//
  auto alreadyImported(fs::path const &filename) noexcept -> bool;
  auto findImport(fs::path const &filename) noexcept
      -> ImportedTranslationUnit *;
  auto addImport(fs::path &&filename,
                 TranslationUnit::Expressions &&expressions) noexcept
      -> ImportedTranslationUnit &;

  //**** Scope interface ****//
  auto localScope() noexcept -> std::shared_ptr<Scope>;
  auto nearestNamedScope() noexcept -> std::shared_ptr<Scope>;
  auto exchangeLocalScope(std::shared_ptr<Scope> scope) noexcept
      -> std::shared_ptr<Scope>;

  void pushScope() noexcept;
  void pushScope(Identifier name) noexcept;
  void popScope() noexcept;

  void unbindScope(Identifier name) noexcept;

  auto declareName(FormalArgument const &argument) noexcept
      -> Result<Bindings::Binding>;
  auto declareName(Identifier name, Attributes attributes,
                   type::Ptr type) noexcept -> Result<Bindings::Binding>;

  auto lookupBinding(Identifier name) noexcept -> Result<Bindings::Binding>;
  auto lookupLocalBinding(Identifier name) noexcept
      -> Result<Bindings::Binding>;
  auto qualifyName(Identifier name) noexcept -> Identifier;

  //**** Use Before Def Interface ****//
  std::optional<Error> bindUseBeforeDef(Identifier undef, Identifier def,
                                        std::shared_ptr<Scope> const &scope,
                                        ast::Ptr p) noexcept;

  std::optional<Error> resolveTypeOfUseBeforeDef(Identifier def_name) noexcept;

  std::optional<Error>
  resolveComptimeValueOfUseBeforeDef(Identifier def_name) noexcept;

  std::optional<Error>
  resolveRuntimeValueOfUseBeforeDef(Identifier def_name) noexcept;

  std::optional<Error>
  resolveForwardDeclarationValueOfUseBeforeDef(Identifier def_name) noexcept;

  //**** BinopTable Interface ****//
  auto createBinop(Token op) noexcept -> BinopTable::Binop;
  auto lookupBinop(Token op) noexcept -> std::optional<BinopTable::Binop>;

  //**** UnopTable Interface ****//
  auto createUnop(Token op) noexcept -> UnopTable::Unop;
  auto lookupUnop(Token op) noexcept -> std::optional<UnopTable::Unop>;

  //**** TypeInterner Interface ****//
  auto getBooleanType() noexcept -> type::Ptr;
  auto getIntegerType() noexcept -> type::Ptr;
  auto getNilType() noexcept -> type::Ptr;

  auto getFunctionType(type::Ptr result_type,
                       type::Function::Arguments argument_types) noexcept
      -> type::Ptr;
  auto getLambdaType(type::Ptr function_type) noexcept -> type::Ptr;

  //**** LLVM interface ****//
  //**** LLVM Helpers *****//
  auto createBasicBlock(llvm::Twine const &name = "") noexcept
      -> llvm::BasicBlock *;

  auto createBasicBlock(llvm::Function *function,
                        llvm::Twine const &name = "") noexcept
      -> llvm::BasicBlock *;

  auto hasInsertionPoint() const noexcept -> bool;
  auto exchangeInsertionPoint(InsertionPoint point = InsertionPoint{}) noexcept
      -> InsertionPoint;

  //**** LLVM StringSet Interface ****//
  auto internString(std::string_view string) -> std::string_view;

  //**** LLVM Module Interface ****//
  auto getLLVMModule() noexcept -> llvm::Module &;

  void printModule(llvm::raw_ostream &OS,
                   llvm::AssemblyAnnotationWriter *AAW = nullptr,
                   bool ShouldPreserveUseListOrder = false,
                   bool IsForDebug = false) noexcept;

  auto getOrInsertGlobal(std::string_view name, llvm::Type *type) noexcept
      -> llvm::GlobalVariable *;

  auto getOrInsertFunction(std::string_view name,
                           llvm::FunctionType *type) noexcept
      -> llvm::FunctionCallee;

  //**** LLVM IRBuilder interface ****//
  // types
  auto getLLVMVoidType() noexcept -> llvm::Type *;
  auto getLLVMNilType() noexcept -> llvm::IntegerType *;
  auto getLLVMBooleanType() noexcept -> llvm::IntegerType *;
  auto getLLVMIntegerType() noexcept -> llvm::IntegerType *;
  auto getLLVMSizeType() noexcept -> llvm::IntegerType *;

  auto getLLVMFunctionType(llvm::Type *result_type,
                           llvm::ArrayRef<llvm::Type *> argument_types) noexcept
      -> llvm::FunctionType *;

  auto getLLVMPointerType() noexcept -> llvm::PointerType *;

  // values
  auto getLLVMNil() noexcept -> llvm::ConstantInt *;
  auto getLLVMBoolean(bool value) noexcept -> llvm::ConstantInt *;
  auto getLLVMInteger(unsigned value) noexcept -> llvm::ConstantInt *;

  // instructions
  auto createLLVMNeg(llvm::Value *right, llvm::Twine const &name = "neg",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  auto createLLVMNot(llvm::Value *right,
                     llvm::Twine const &name = "not") noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#add-instruction */
  auto createLLVMAdd(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "add",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#sub-instruction */
  auto createLLVMSub(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "sub",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#mul-instruction */
  auto createLLVMMul(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "mul",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#sdiv-instruction */
  auto createLLVMSDiv(llvm::Value *left, llvm::Value *right,
                      llvm::Twine const &name = "sdiv",
                      bool is_exact = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#srem-instruction */
  auto createLLVMSRem(llvm::Value *left, llvm::Value *right,
                      llvm::Twine const &name = "srem") noexcept
      -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#icmp-instruction */
  auto createLLVMICmpEQ(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "eq") noexcept
      -> llvm::Value *;

  auto createLLVMICmpNE(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "ne") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSGT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "sgt") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSGE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "sge") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSLT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "slt") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSLE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "sle") noexcept
      -> llvm::Value *;

  auto createLLVMAnd(llvm::Value *left, llvm::Value *right,
                     const llvm::Twine &name = "and") noexcept -> llvm::Value *;
  auto createLLVMOr(llvm::Value *left, llvm::Value *right,
                    const llvm::Twine &name = "or") noexcept -> llvm::Value *;

  // memory related instructions
  auto createLLVMAlloca(llvm::Type *type, llvm::Value *array_length = nullptr,
                        const llvm::Twine &name = "alloca") noexcept
      -> llvm::AllocaInst *;

  auto createLLVMLoad(llvm::Type *type, llvm::Value *source) noexcept
      -> llvm::LoadInst *;

  auto createLLVMStore(llvm::Value *source, llvm::Value *target,
                       bool is_volatile = false) noexcept -> llvm::StoreInst *;

  // function related instructions
  auto createLLVMCall(llvm::Function *callee,
                      llvm::ArrayRef<llvm::Value *> arguments,
                      llvm::Twine const &name = "",
                      llvm::MDNode *fp_math_tag = nullptr) noexcept
      -> llvm::CallInst *;

  auto createLLVMCall(llvm::FunctionCallee callee,
                      llvm::ArrayRef<llvm::Value *> arguments,
                      llvm::Twine const &name = "",
                      llvm::MDNode *fp_math_tag = nullptr) noexcept
      -> llvm::CallInst *;

  auto createLLVMReturn(llvm::Value *value = nullptr) noexcept
      -> llvm::ReturnInst *;

  // casting related instructions
  auto createLLVMTrunc(llvm::Value *value, llvm::Type *destination_type,
                       const llvm::Twine &name = "trunc") noexcept
      -> llvm::Value *;

  auto createLLVMZExt(llvm::Value *value, llvm::Type *destination_type,
                      const llvm::Twine &name = "zext") noexcept
      -> llvm::Value *;

  auto createLLVMSExt(llvm::Value *value, llvm::Type *destination_type,
                      const llvm::Twine &name = "sext") noexcept
      -> llvm::Value *;

  auto createLLVMBitcast(llvm::Value *value, llvm::Type *destination_type,
                         const llvm::Twine &name = "bitcast") noexcept
      -> llvm::Value *;
};
} // namespace mint
