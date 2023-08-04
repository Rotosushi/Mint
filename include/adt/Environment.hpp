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
#include "adt/TypeInterner.hpp"
#include "adt/UnopTable.hpp"
#include "adt/UseBeforeDefMap.hpp"
#include "ast/Ast.hpp"
#include "scan/Parser.hpp"

namespace mint {
class Environment {
  DirectorySearcher m_directory_searcher;
  ImportSet m_import_set;
  IdentifierSet m_identifier_set;
  TypeInterner m_type_interner;
  // AstAllocator ast_allocator;
  BinopTable m_binop_table;
  UnopTable m_unop_table;
  UseBeforeDefMap m_use_before_def_map;
  //  #NOTE: since we only have std::weak_ptrs
  //  back up the tree of scopes, we must hold
  //  a shared_ptr to the top of the tree.
  //  so when we traverse to a new scope,
  //  all of the scopes stay alive.
  std::shared_ptr<Scope> m_global_scope;
  std::shared_ptr<Scope> m_local_scope;
  Parser m_parser;
  std::vector<ast::Ptr> m_module;

  std::istream *m_input;
  std::ostream *m_output;
  std::ostream *m_error_output;
  std::ostream *m_log_output;

  std::unique_ptr<llvm::LLVMContext> m_llvm_context;
  std::unique_ptr<llvm::Module> m_llvm_module;
  std::unique_ptr<llvm::IRBuilder<>> m_llvm_ir_builder;
  llvm::TargetMachine *m_llvm_target_machine;
  // llvm::Function *current_llvm_function;

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

  // #NOTE:
  // string is some quoted text from the source code: "\"...\""
  // return the same string without quotes "..."
  static auto getTextFromTextLiteral(std::string_view string) noexcept
      -> std::string_view;

  auto repl(bool do_print) noexcept -> int;
  auto compile(std::filesystem::path file) noexcept -> int;

  void printErrorWithSource(Error const &error) const noexcept;
  void printErrorWithSource(Error const &error,
                            Parser const &parser) const noexcept;

  auto localScope() noexcept -> std::shared_ptr<Scope>;
  auto exchangeLocalScope(std::shared_ptr<Scope> scope) noexcept
      -> std::shared_ptr<Scope>;

  //   intended to be called when processing an ast::Function,
  //   or ast::Block
  //   such that lookup and binding occurs within the local
  //   scope of the function.
  //   note the lack of a scope name, and thus no ability to
  //   bind the scope to the current scope. thus anonymous scopes
  //   are only alive as long as they are the local_scope.
  //
  //   this behavior is intended for functions, whose names
  //   only need to be alive for the time functions are being
  //   evaluated. but not for modules, as a modules names
  //   must be alive for the lifetime of a given program.
  void pushScope() noexcept;

  //   called when processing an Ast::Module,
  //   such that lookup and binding occurs within the
  //   local scope of the module.
  void pushScope(Identifier name) noexcept;

  // traverse up the scope tree one level.
  void popScope() noexcept;

  //**** Environment member interfaces ****//
  //**** global "module" interface ****//
  void addAstToModule(ast::Ptr ast) noexcept;

  //**** DirectorySearcher interface ****//
  void appendDirectory(fs::path file) noexcept;

  auto fileExists(fs::path file) noexcept -> bool;

  auto fileSearch(fs::path file) noexcept -> std::optional<std::fstream>;

  //**** Identifier Set Interface ****//
  auto getIdentifier(std::string_view name) noexcept -> Identifier;

  //**** ImportSet interface ****//
  auto alreadyImported(fs::path const &filename) noexcept -> bool;

  void addImport(fs::path const &filename) noexcept;

  //**** Scope interface ****//
  // called when we fail to create a module, so we
  // don't partially define a module within the current
  // namespace.
  void unbindScope(Identifier name) noexcept;

  auto bindName(Identifier name, Attributes attributes, type::Ptr type,
                ast::Ptr comptime_value, llvm::Value *runtime_value) noexcept
      -> mint::Result<mint::Bindings::Binding>;

  auto partialBindName(Identifier name, Attributes attributes,
                       type::Ptr type) noexcept
      -> mint::Result<mint::Bindings::Binding>;

  auto lookupBinding(Identifier name) noexcept
      -> mint::Result<mint::Bindings::Binding>;
  auto lookupLocalBinding(Identifier name) noexcept
      -> mint::Result<mint::Bindings::Binding>;
  auto qualifyName(Identifier name) noexcept -> Identifier;

  //**** Use Before Def Interface ****/
  std::optional<Error> bindUseBeforeDef(Error const &error,
                                        ast::Ptr ast) noexcept;

  std::optional<Error> resolveTypeOfUseBeforeDef(Identifier def_name) noexcept;

  std::optional<Error>
  resolveComptimeValueOfUseBeforeDef(Identifier def_name) noexcept;

  std::optional<Error>
  resolveRuntimeValueOfUseBeforeDef(Identifier def_name) noexcept;

  //**** BinopTable Interface ****/
  auto createBinop(Token op) noexcept -> BinopTable::Binop;
  auto lookupBinop(Token op) noexcept -> std::optional<BinopTable::Binop>;

  //**** UnopTable Interface ****/
  auto createUnop(Token op) noexcept -> UnopTable::Unop;
  auto lookupUnop(Token op) noexcept -> std::optional<UnopTable::Unop>;

  //**** TypeInterner Interface ****/
  auto getBooleanType() noexcept -> type::Boolean const *;
  auto getIntegerType() noexcept -> type::Integer const *;
  auto getNilType() noexcept -> type::Nil const *;

  auto getFunctionType(type::Ptr result_type,
                       std::vector<type::Ptr> argument_types) noexcept
      -> type::Function const *;

  //**** LLVM interface ****//
  //**** LLVM Helpers *****//
  auto createQualifiedNameForLLVM(Identifier name) noexcept -> Identifier;

  auto createBasicBlock(llvm::Twine const &name = "") noexcept
      -> llvm::BasicBlock *;

  auto createBasicBlock(llvm::Function *function,
                        llvm::Twine const &name = "") noexcept
      -> llvm::BasicBlock *;

  auto exchangeInsertionPoint(InsertionPoint point = InsertionPoint{}) noexcept
      -> InsertionPoint;

  //**** LLVM Module Interface ****//
  auto getOrInsertGlobal(std::string_view name, llvm::Type *type) noexcept
      -> llvm::GlobalVariable *;

  auto getOrInsertFunction(std::string_view name,
                           llvm::FunctionType *type) noexcept
      -> llvm::FunctionCallee;

  /**** LLVM IRBuilder interface ****/ //
  // types
  auto getLLVMNilType() noexcept -> llvm::IntegerType *;
  auto getLLVMBooleanType() noexcept -> llvm::IntegerType *;
  auto getLLVMIntegerType() noexcept -> llvm::IntegerType *;

  auto getLLVMFunctionType(llvm::Type *result_type,
                           llvm::ArrayRef<llvm::Type *> argument_types) noexcept
      -> llvm::FunctionType *;

  // values
  auto getLLVMNil() noexcept -> llvm::ConstantInt *;
  auto getLLVMBoolean(bool value) noexcept -> llvm::ConstantInt *;
  auto getLLVMInteger(int value) noexcept -> llvm::ConstantInt *;

  // instructions
  auto createLLVMNeg(llvm::Value *right, llvm::Twine const &name = "",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  auto createLLVMNot(llvm::Value *right, llvm::Twine const &name = "") noexcept
      -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#add-instruction */
  auto createLLVMAdd(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#sub-instruction */
  auto createLLVMSub(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#mul-instruction */
  auto createLLVMMul(llvm::Value *left, llvm::Value *right,
                     llvm::Twine const &name = "",
                     bool no_unsigned_wrap = false,
                     bool no_signed_wrap = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#sdiv-instruction */
  auto createLLVMSDiv(llvm::Value *left, llvm::Value *right,
                      llvm::Twine const &name = "",
                      bool is_exact = false) noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#srem-instruction */
  auto createLLVMSRem(llvm::Value *left, llvm::Value *right,
                      llvm::Twine const &name = "") noexcept -> llvm::Value *;

  /* https://llvm.org/docs/LangRef.html#icmp-instruction */
  auto createLLVMICmpEQ(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "") noexcept -> llvm::Value *;

  auto createLLVMICmpNE(llvm::Value *left, llvm::Value *right,
                        const llvm::Twine &name = "") noexcept -> llvm::Value *;

  auto createLLVMICmpSGT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSGE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSLT(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept
      -> llvm::Value *;

  auto createLLVMICmpSLE(llvm::Value *left, llvm::Value *right,
                         const llvm::Twine &name = "") noexcept
      -> llvm::Value *;

  auto createLLVMAnd(llvm::Value *left, llvm::Value *right,
                     const llvm::Twine &name = "") noexcept -> llvm::Value *;
  auto createLLVMOr(llvm::Value *left, llvm::Value *right,
                    const llvm::Twine &name = "") noexcept -> llvm::Value *;

  auto createLLVMLoad(llvm::Type *type, llvm::Value *source) noexcept
      -> llvm::Value *;

  auto createLLVMReturn(llvm::Value *value = nullptr) noexcept
      -> llvm::ReturnInst *;
};
} // namespace mint
