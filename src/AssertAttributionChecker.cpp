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
      DiagnosticIDs::Error, "Forbidden attribution in assert expression, "
                            "causing the result to always be true");
  
  CallExpr *funcCall = const_cast<CallExpr*>(Result.Nodes.getNodeAs<CallExpr>("funcCall"));
  
  if (!funcCall) {
    return;
  }
  
  // Evaluate first parameter from the call list
  // to see if it's a BinaryOperator
  
  Expr *exprArg = funcCall->getArg(0);
  
  // Ignore all implicit castings that are done
  exprArg = exprArg->IgnoreImplicit();
  
  // Only evaluate the first argument from the CallExpr argument list.
  // The syntax of the call argument will look similar with:
  // !(Expr)
  // First there is an UnaryOperator followed by ParenExpr then check for
  // a binary operator of type BO_Assign and if so trigger an error message.
  const UnaryOperator *unOp = dyn_cast_or_null<UnaryOperator>(exprArg);
  
  if (!unOp) {
    return;
  }
  
  Expr *unOpResExpr = unOp->getSubExpr();
  
  if (!unOpResExpr) {
    return;
  }
  
  // Strip off any ParenExpr or ImplicitCastExprs
  unOpResExpr = unOpResExpr->IgnoreParenImpCasts();
  
  if (const BinaryOperator *binOp = dyn_cast_or_null<BinaryOperator>(unOpResExpr)) {
    if (binOp->getOpcode() == BO_Assign) {
      Diag.Report(funcCall->getLocStart(), assignInsteadOfComp);
    }
  }
}
