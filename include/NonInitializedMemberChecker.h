/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NonInitializedMemberChecker_h__
#define NonInitializedMemberChecker_h__


#include "ClangDefines.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace DiagnosticsMatcher {
  class NonInitializedMemberChecker : public MatchFinder::MatchCallback {
  public:
    void run(const MatchFinder::MatchResult &Result) override;

  private:
    const uint8_t MAX_DEPTH = 100;
    enum InitFlag : uint8_t {
      NotInit,
      InitByCtor,
      InitByFunc
    };

    void runThroughDefaultFunctions(
        CallExpr *callExpr,
        std::unordered_map<std::string, InitFlag>& variablesMap,
        std::unordered_map<std::string, std::string>& resolverMap, InitFlag flag);
    void
    checkValueDecl(Expr *exp,
        std::unordered_map<std::string, InitFlag>& variablesMap,
        std::unordered_map<std::string, std::string>& resolverMap, InitFlag flag);
    StringRef
    resolveMapVar(std::unordered_map<std::string, std::string> &resolverMap,
                  StringRef varName);
    void updateVarMap(std::unordered_map<std::string, InitFlag>& variablesMap,
                      StringRef varName, InitFlag flag);
    bool buildResolverMap(
        CallExpr *callExp,
        std::unordered_map<std::string, std::string> &resolverMap,
        std::unordered_map<std::string, std::string> &newResolverMap);
    bool getVarNameFromExprWithThisCheck(Expr *expr, StringRef &varName);
    void
    evaluateExpression(Stmt *stmtExpr,
      std::unordered_map<std::string, InitFlag>& variablesMap,
      std::unordered_map<std::string, std::string>& resolverMap, uint8_t depth,
      InitFlag flag);
    void
    evaluateChildrenIfNotNull(Stmt *stmtExpr,
      std::unordered_map<std::string, InitFlag>& variablesMap,
      std::unordered_map<std::string, std::string>& resolverMap, uint8_t depth,
      InitFlag flag);
  };
}

#endif
