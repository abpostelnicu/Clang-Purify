//
//  RestrictedOverloadedOperatorChecker.h
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 11/07/16.
//
//

#ifndef RestrictedOverloadedOperatorChecker_h
#define RestrictedOverloadedOperatorChecker_h

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
#include <stdio.h>
#include "ClangDefines.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace DiagnosticsMatcher {
  class RestrictedOverloadedOperatorChecker : public MatchFinder::MatchCallback {
  public:
    virtual void run(const MatchFinder::MatchResult &Result);
  private:
    struct SearchVars {
      std::string sizeName;
      std::string ptrName;
    };
    bool findMatch(Stmt *stmt, SearchVars &vars);
    StringRef getVarNameFromExpr(Expr *expr);
  };
}

#endif /* RestrictedOverloadedOperatorChecker_h */
