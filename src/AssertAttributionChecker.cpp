//
//  AssertChecker.cpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 30/06/16.
//
//

#include "AssertAttributionChecker.h"

void DiagnosticsMatcher::AssertAttributionChecker::run(const MatchFinder::MatchResult &Result) {
  
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned assignInsteadOfComp = Diag.getDiagnosticIDs()->getCustomDiagID(
                                    DiagnosticIDs::Error,
                                    "The variable will be assigned the value that it was intended"\
                                    "to compare against, causing the result to always be true");
  
  CallExpr *funcCall = const_cast<CallExpr*>(Result.Nodes.getNodeAs<CallExpr>("funcCall"));
  
  if (!funcCall) {
    return;
  }
  
  // Evaluate first parameter from the call list
  // to see if it's a BinaryOperator
  
  Expr *exprArg = funcCall->getArg(0);
  
  if (!exprArg) {
    return;
  }
  
  exprArg = exprArg->IgnoreImplicit();
  
  if (BinaryOperator *binOp = dyn_cast_or_null<BinaryOperator>(exprArg)) {
    // If it's an BO_Assign than signal the problem
    if (binOp->getOpcode() == BO_Assign) {
      Diag.Report(funcCall->getLocStart(), assignInsteadOfComp);
    }
  }
}
