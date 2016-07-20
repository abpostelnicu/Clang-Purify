//
//  RestrictedOverloadedOperatorChecker.cpp
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 11/07/16.
//
//

#include "RestrictedOverloadedOperatorChecker.h"

StringRef DiagnosticsMatcher::RestrictedOverloadedOperatorChecker::getVarNameFromExpr(Expr *expr) {
  expr = expr->IgnoreImplicit();
  
  DeclRefExpr *declExpr = dyn_cast_or_null<DeclRefExpr>(expr);
  if (!declExpr) {
    return StringRef();
  }
  
  if (!declExpr->getDecl()) {
    return StringRef();
  }
  
  return declExpr->getDecl()->getName();
}

bool DiagnosticsMatcher::RestrictedOverloadedOperatorChecker::findMatch(clang::Stmt *stmtExpr, SearchVars &vars) {
  bool match = false;
  stmtExpr = stmtExpr->IgnoreImplicit();
  
  if (CompoundStmt *cmpdStmt = dyn_cast_or_null<CompoundStmt>(stmtExpr)) {
    // This Stmt is actually CompoundStmt then loop through all it's children
    for (auto child : cmpdStmt->children()) {
      match |= findMatch(child, vars);
    }
  } else if (IfStmt *ifStmt = dyn_cast_or_null<IfStmt>(stmtExpr)) {
    // If this is an if statement go through then and else statements,
    // if else statement is not present just skip it
    Stmt *thenStmt = ifStmt->getThen();
    Stmt *elseStmt = ifStmt->getElse();
    
    if (thenStmt) {
      match |= findMatch(thenStmt, vars);
      if (elseStmt) {
        match |= findMatch(elseStmt, vars);
      }
    }
  } else if (CallExpr *funcCall = dyn_cast_or_null<CallExpr>(stmtExpr)) {
    // The pattern here is memset(ptr, 0, size) where size should be the same variable
    // as the one passed to operator new(size_t size) and ptr is returned
    FunctionDecl *method = funcCall->getDirectCallee();
    if (!method) {
      return false;
    }
    if (!method->hasBody() && method->getDeclName().isIdentifier() &&
        !strcmp("memset", method->getName().data())) {
      // If 3rd parameter name is the same with vars.sizeName maybe we have a match
      if (funcCall->getNumArgs() != 3) {
        return false;
      }
      Expr *sizeExpr = funcCall->getArg(2);
      
      if (!sizeExpr) {
        return false;
      }
      
      if (getVarNameFromExpr(sizeExpr) != vars.sizeName) {
        return false;
      }
      
      // Assign the name of the first parameter to vars.ptrName
      Expr *ptrExpr = funcCall->getArg(0);
      
      if (!ptrExpr) {
        return false;
      }
      
      vars.ptrName = getVarNameFromExpr(ptrExpr);
    }
  } else if (ReturnStmt *returnStmt = dyn_cast_or_null<ReturnStmt>(stmtExpr)) {
    // Compare the name of the return ptr to vars.ptrName since operator new must
    // return a pointer to an allocated memory
    Expr *retExpr = returnStmt->getRetValue();
    
    if (!retExpr) {
      return false;
    }
    
    match |= vars.ptrName.size() && getVarNameFromExpr(retExpr) == vars.ptrName;
  }
  return match;
}

void DiagnosticsMatcher::RestrictedOverloadedOperatorChecker::run(const MatchFinder::MatchResult &Result) {
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned newOperatorOverloadedID = Diag.getDiagnosticIDs()->getCustomDiagID(
      DiagnosticIDs::Error,
      "Not allowed operator new overloading");
  SearchVars vars;
  const CXXMethodDecl *method = Result.Nodes.getNodeAs<CXXMethodDecl>("methodDecl");
  
  if (!method) {
    return;
  }

  // As we also check for uninitialised variables and we scan
  if (method->getDeclName().getCXXOverloadedOperator() != OO_New) {
    return;
  }
  
  // This is when definition is done outside of declaration
  // loop through the redecls and get the one that is different
  // than the current ctor since it will be the definition.
  if (!method->isThisDeclarationADefinition()) {
    for (auto mthdList : method->redecls()) {
      CXXMethodDecl *mthd = dyn_cast_or_null<CXXMethodDecl>(mthdList);
      if (mthd && mthd->isThisDeclarationADefinition()) {
        method = mthd;
        break;
      }
    }
  }
  
  if (!method->isThisDeclarationADefinition()) {
    return;
  }
  
  // from operator new get the name of first parameter size_t size
  auto param = method->getParamDecl(0);
  
  if (!param) {
    return;
  }
  vars.sizeName = param->getName();
  
  if (findMatch(method->getBody(), vars)) {
    Diag.Report(method->getLocation(), newOperatorOverloadedID);
  }
}
