#ifndef ClangParser_h
#define ClangParser_h

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Support/Host.h"

class ClangParser
{
  clang::CompilerInstance m_CompilerInstance;
  clang::VerifyDiagnosticConsumer *m_VerifDiagConsumer;
  std::shared_ptr<clang::TargetOptions> m_TargetOptions;
  
  void prepareCompilerforFile(const char* szSourceCodeFilePath);

public:
  ClangParser();
  ~ClangParser();

  bool parseAST(const char* szSourceCodeFilePath, clang::ast_matchers::MatchFinder finder);

  // Executes CodeGenAction and returns the pointer (caller should own and delete it)
  // Returns NULL on failure (in case of any compiler errors etc.)
  clang::CodeGenAction* emitLLVM(const char* szSourceCodeFilePath);

  clang::ASTContext& GetASTContext() { return m_CompilerInstance.getASTContext(); }
};

#endif // ClangParser_h