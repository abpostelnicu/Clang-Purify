//
//  AssertChecker.hpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 30/06/16.
//
//

#ifndef AssertAttributionChecker_h
#define AssertAttributionChecker_h

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
#include "Debug.h"
#include "ClangDefines.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace DiagnosticsMatcher {
  class AssertAttributionChecker : public MatchFinder::MatchCallback {
  public:
    virtual void run(const MatchFinder::MatchResult &Result);
  };
}
#endif /* AssertChecker_h */
