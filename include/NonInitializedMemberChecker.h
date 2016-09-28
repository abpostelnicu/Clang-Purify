#ifndef NonInitializedMemberChecker_h
#define NonInitializedMemberChecker_h

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Support/Host.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <ostream>
#include "Debug.h"
#include "ClangDefines.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace DiagnosticsMatcher {
  class NonInitializedMemberChecker : public MatchFinder::MatchCallback {
  public:
    virtual void run(const MatchFinder::MatchResult &Result);
  private:
    // Max depth to witch we perform analysis on the AST
    const uint8_t MaxDepth = 100;
    enum InitFlag : uint8_t {
      NotInit,
      InitByCtor,
      InitByFunc
    };
    void runThroughDefaultFunctions(CallExpr *callExpr,
                                    std::unordered_map<std::string, InitFlag>& variablesMap,
                                    std::unordered_map<std::string, std::string>& resolverMap, InitFlag flag);
    void checkValueDecl(Expr *exp,
                        std::unordered_map<std::string, InitFlag>& variablesMap,
                        std::unordered_map<std::string, std::string>& resolverMap, InitFlag flag);
    StringRef resolveMapVar(
                            std::unordered_map<std::string, std::string>& resolverMap, StringRef varName);
    void updateVarMap(std::unordered_map<std::string, InitFlag>& variablesMap,
                      StringRef varName, InitFlag flag);
    bool buildResolverMap(CallExpr *callExp,
                          std::unordered_map<std::string, std::string>& resolverMap,
                          std::unordered_map<std::string, std::string>& newResolverMap);
    bool getVarNameFromExprWithThisCheck(Expr *expr, StringRef& varName);
    void evaluateExpression(Stmt *stmtExpr,
                            std::unordered_map<std::string, InitFlag>& variablesMap,
                            std::unordered_map<std::string, std::string>& resolverMap, uint8_t depth,
                            InitFlag flag);
  };
}

#endif // NonInitializedMemberChecker_h