//
//  AssertChecker.cpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 30/06/16.
//
//

#include "AssertAssignmentChecker.h"
#include "clang/AST/EvaluatedExprVisitor.h"

bool HasSideEffectAssignment(const Expr *expr) {
  
  if (auto opCallExpr = dyn_cast<CXXOperatorCallExpr>(expr)) {
      auto binOp = opCallExpr->getOperator();
    if (binOp == OO_Equal || (binOp >= OO_PlusEqual && binOp <= OO_PipeEqual)) {
        return true;
    }
  } else if (auto binOpExpr = dyn_cast_or_null<BinaryOperator>(expr)) {
    if (binOpExpr->isAssignmentOp()) {
      return true;
    }
  }

  // Recurse to children.
  for (const Stmt *SubStmt : expr->children()) {
    auto childExpr = dyn_cast_or_null<Expr>(SubStmt);
    if (SubStmt && HasSideEffectAssignment(childExpr))
      return true;
  }
  
  return false;
}

void DiagnosticsMatcher::AssertAssignmentChecker::run(const MatchFinder::MatchResult &Result) {
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned assignInsteadOfComp = Diag.getDiagnosticIDs()->getCustomDiagID(
      DiagnosticIDs::Error, "Forbidden assignment in assert expression");
  const CallExpr *funcCall = Result.Nodes.getNodeAs<CallExpr>("funcCall");
  
  if (funcCall && HasSideEffectAssignment(funcCall)) {
    Diag.Report(funcCall->getLocStart(), assignInsteadOfComp);
  }
}