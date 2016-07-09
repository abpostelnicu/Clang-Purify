//
//  AssertChecker.cpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 30/06/16.
//
//

#include "AssertAssignmentChecker.h"

void DiagnosticsMatcher::AssertAssignmentChecker::run(const MatchFinder::MatchResult &Result) {
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned assignInsteadOfComp = Diag.getDiagnosticIDs()->getCustomDiagID(
      DiagnosticIDs::Error, "Forbidden assignment in assert expression");
  
  bool foundError = false;
  const CallExpr *funcCall = Result.Nodes.getNodeAs<CallExpr>("funcCall");
  
  if (!funcCall) {
    return;
  }
  
  // Evaluate first parameter from the call list
  Expr *exprArg = const_cast<Expr*>(funcCall->getArg(0));
  
  // Ignore all implicit castings that are done
  exprArg = exprArg->IgnoreImplicit();
  
  // The syntax of the call argument will look similar with:
  // !(Expr)
  // First there is an UnaryOperator followed by ParenExpr
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
  
  // For normal primitives there should follow a BinaryOperator of BO_Assign
  // type but for classes that have overloaded "operator =" there will be a
  // CXXMemberCallExpr with an implicit argument of CXXOperatorCallExpr
  if (const BinaryOperator *binOp = dyn_cast_or_null<BinaryOperator>(unOpResExpr)) {
    if (binOp->getOpcode() == BO_Assign) {
      foundError = true;
    }
  } else if (const CXXMemberCallExpr *callExpr = dyn_cast_or_null<CXXMemberCallExpr>(unOpResExpr)) {
    Expr *subExpr = const_cast<Expr*>(callExpr->getImplicitObjectArgument());
    
    if (!subExpr) {
      return;
    }
    
    subExpr = subExpr->IgnoreParenImpCasts();
    
    if (CXXOperatorCallExpr *opCall = dyn_cast_or_null<CXXOperatorCallExpr>(subExpr)) {
      if (opCall->getOperator() == OO_Equal) {
        foundError = true;
      }
    }
  }
  
  if (foundError) {
    Diag.Report(funcCall->getLocStart(), assignInsteadOfComp);
  }
}
