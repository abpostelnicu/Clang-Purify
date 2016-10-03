//
//  OverridebaseCallCheck.cpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 29/09/2016.
//
//

#include "OverrideBaseCallCheck.h"

bool DiagnosticsMatcher::OverrideBaseCallCheck::isSuitable(const CXXMethodDecl *method) {
  return true;
}

void DiagnosticsMatcher::OverrideBaseCallCheck::evaluateExpression(Stmt *stmtExpr,
                                                                   const CXXRecordDecl *childClass,
                                                                   std::list<const CXXMethodDecl*> &methodList) {
  // In order to optimize seach process, continue while we have methods in our
  // list
  if (!methodList.size()) {
    return;
  }
  stmtExpr = stmtExpr->IgnoreImplicit();
    
  if (BinaryOperator *binOp = dyn_cast_or_null<BinaryOperator>(stmtExpr)) {
    // If BinaryOperator check the left and right operands - left for value decl
    // right for function call
    if (binOp->getOpcode() == BO_Assign) {
      Expr *exprLeft = binOp->getLHS();
      if (!exprLeft) {
        return;
      }
      evaluateExpression(exprLeft, childClass, methodList);
      
      Expr *exprRight = binOp->getRHS();
      if (!exprRight) {
        return;
      }
      evaluateExpression(exprRight, childClass, methodList);
    }
  } else if (IfStmt *ifStmt = dyn_cast_or_null<IfStmt>(stmtExpr)){
    // If this is an if statement go through then and else statements,
    // if else statement is not present just skip it
    Stmt *thenStmt = ifStmt->getThen();
    Stmt *elseStmt = ifStmt->getElse();
    
    if (thenStmt) {
      evaluateExpression(ifStmt, childClass, methodList);
    }
    if (elseStmt) {
      evaluateExpression(elseStmt, childClass, methodList);
    }
  } if (ForStmt *forStmt = dyn_cast_or_null<ForStmt>(stmtExpr)) {
    // If this is a ForStmt go through it's body Stmt
    Stmt *bodyFor = forStmt->getBody();
    if (bodyFor) {
      evaluateExpression(bodyFor, childClass, methodList);
    }
  }  else if (WhileStmt *whileStmt = dyn_cast_or_null<WhileStmt>(stmtExpr)) {
    // If this is a WhileStmt go through it's body Stmt
    Stmt *bodyWhile = whileStmt->getBody();
    if (bodyWhile) {
      evaluateExpression(bodyWhile, childClass, methodList);
    }
  } else if (DoStmt *doStmt = dyn_cast_or_null<DoStmt>(stmtExpr)) {
    // If this is a DoStmt go through it's body Stmt
    Stmt *bodyDo = doStmt->getBody();
    if (bodyDo) {
      evaluateExpression(bodyDo, childClass, methodList);
    }
  } else if (CompoundStmt *cmpdStmt = dyn_cast_or_null<CompoundStmt>(stmtExpr)) {
    // This Stmt is actually CompoundStmt then loop through all it's children
    for (auto child : cmpdStmt->children()) {
      evaluateExpression(child, childClass, methodList);
    }
  } else if (CXXMemberCallExpr *memberFuncCall = dyn_cast_or_null<CXXMemberCallExpr>(stmtExpr)) {
    CXXMethodDecl *method = dyn_cast_or_null<CXXMethodDecl>(memberFuncCall->getDirectCallee());
    if (!method) {
      return;
    }
    
    // The candidate method calls must be subject to these criteria:
    //  1. underlying object parent must be different than the current one
    //  2. underlying method definition must pass isSuitable
    if (method->getParent() != childClass &&
        isSuitable(method)) {
      methodList.remove(method);
    }
  }
}

void DiagnosticsMatcher::OverrideBaseCallCheck::run(const MatchFinder::MatchResult &Result) {
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned overrideBaseCallCheckID = Diag.getDiagnosticIDs()->getCustomDiagID(
                                                                             DiagnosticIDs::Error,
                                                                             "Base method %0 is not called in overriden method from %1");
  const CXXRecordDecl *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("class");
  
  if (!decl) {
    return;
  }
  
  // Loop through the methods and look for the ones that are
  // overriden.
  for (auto method : decl->methods()) {
    
    // If this method doesn't override other methods, continue
    // to the next one.
    if (!method->size_overridden_methods()) {
      continue;
    }
    
    std::list<const CXXMethodDecl*> methodsList;
    // For each overriden method push it to a list if it mets our
    // criteria 
    for (auto baseMethod : method->overridden_methods()) {
      if (isSuitable(baseMethod)) {
        methodsList.push_back(baseMethod);
      }
    }
    
    // If no method has been found than no annotation was used
    // so checking for direct callee is not needed
    if (!methodsList.size()) {
      continue;
    }
    
    // Get the definition for this declaration
    if (!method->isThisDeclarationADefinition()) {
      for (auto methodFromList : method->redecls()) {
        CXXMethodDecl *mthd = dyn_cast_or_null<CXXMethodDecl>(methodFromList);
        
        if (mthd && mthd->isThisDeclarationADefinition()) {
          method = mthd;
          break;
        }
      }
    }
    
    // If we didn't find a definition, skip this declaration.
    if (!method->isThisDeclarationADefinition()) {
      continue;
    }
    
    // Loop through the body of our method and search for calls to
    // overriden methods
    evaluateExpression(method->getBody(), decl, methodsList);
    
    // If methodsList is empty continue to the next method declaration
    if (!methodsList.size()) {
      continue;
    }
    
    // This means that there are methods that have not been called
    // so start trigger errors
    for (auto baseMethod : methodsList) {
      Diag.Report(method->getLocation(), overrideBaseCallCheckID)
      << baseMethod->getName() << method;
    }
  }
}
