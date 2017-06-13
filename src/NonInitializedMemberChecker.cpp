/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "NonInitializedMemberChecker.h"
#include "AstCustomMatchers.h"

inline bool hasCustomAnnotation(const Decl *D, StringRef Spelling) {
  iterator_range<specific_attr_iterator<AnnotateAttr>> Attrs =
      D->specific_attrs<AnnotateAttr>();

  for (AnnotateAttr *Attr : Attrs) {
    if (Attr->getAnnotation() == Spelling) {
      return true;
    }
  }

  return false;
}

StringRef DiagnosticsMatcher::NonInitializedMemberChecker::resolveMapVar(
    std::unordered_map<std::string, std::string> &resolverMap,
    StringRef varName) {
  auto varFromMap = resolverMap.find(varName);
  if (varFromMap != resolverMap.end()) {
    // variable found in resolve map that means we must use it's corespondent
    // -> second in variablesMap
    return varFromMap->second;
  }
  return StringRef();
}

bool DiagnosticsMatcher::NonInitializedMemberChecker::getVarNameFromExprWithThisCheck(
    Expr *expr, StringRef &varName) {
  expr = expr->IgnoreImplicit();
  MemberExpr *memberExpr = dyn_cast_or_null<MemberExpr>(expr);
  if (memberExpr && isa<CXXThisExpr>(memberExpr->getBase())) {
    // case 1 - variable from this
    if (!memberExpr->getMemberDecl()) {
      varName = StringRef();
      return false;
    }
    varName = memberExpr->getMemberDecl()->getName();
    return true;
  }
  // case 2 - normal variable referencing
  DeclRefExpr *declExpr = dyn_cast_or_null<DeclRefExpr>(expr);
  if (declExpr) {
    if (!declExpr->getDecl()) {
      return false;
    }
    varName = declExpr->getDecl()->getName();
  } else {
    // case 3 - unary operator &
    UnaryOperator *unaryOp = dyn_cast_or_null<UnaryOperator>(expr);
    if (unaryOp) {
      if (unaryOp->getOpcode() != UO_AddrOf &&
          unaryOp->getOpcode() != UO_Deref) {
        return false;
      }
      Expr *varExp = unaryOp->getSubExpr();
      if (!varExp) {
        return false;
      }
      return getVarNameFromExprWithThisCheck(varExp, varName);
    }
  }
  return false;
}

bool DiagnosticsMatcher::NonInitializedMemberChecker::buildResolverMap(
    CallExpr *callExpr,
    std::unordered_map<std::string, std::string> &resolverMap,
    std::unordered_map<std::string, std::string> &newResolverMap) {

  // use callExp to see what it needs to be pushed by address or reference,
  // if we are in any of these 2 situations lookup variable in both resolveMap
  // and variablesMap in order to have a direct corellation in the new function
  // of passed variables directly from variablesMap
  // It's costly but in this way we can avoid a push/ pop algorithm where
  // at the begining of the function we would have pushed the needed variables
  // from resoveMap to newResolveMap and at the end of the function we would
  // have popped them back thus needing to have more maps for tracking results
  // from called function

  FunctionDecl *method = callExpr->getDirectCallee();
  // optimization for extern function to limit their call only if their,
  // arguments are passed by address or reference and are already referenced in
  // resolveMap
  bool isInteresting = false;

  if (!method) {
    return false;
  }

  for (unsigned i = 0; i < method->getNumParams(); i++) {
    auto *param = method->getParamDecl(i);

    if (param->getType()->isAnyPointerType() ||
        param->getType()->isReferenceType()) {

      Expr *argument = callExpr->getArg(i);
      assert(argument);

      StringRef expVarName;
      bool isFromThis = getVarNameFromExprWithThisCheck(argument, expVarName);

      if (expVarName.size()) {
        // if variable is member of this do not check it in resolverMap
        if (isFromThis) {
          newResolverMap[param->getName()] = expVarName;
          isInteresting = true;
        } else {
          // valid variable name - check to see if it's present in resolverMap
          auto varFromMap = resolverMap.find(expVarName);
          if (varFromMap != resolverMap.end()) {
            newResolverMap[param->getName()] = varFromMap->second;
            isInteresting = true;
          }
        }
      }
    }
  }
  return isInteresting;
}

void DiagnosticsMatcher::NonInitializedMemberChecker::updateVarMap(
    std::unordered_map<std::string, InitFlag> &variablesMap, StringRef varName,
    InitFlag flag) {
  // Only set it if it's unset since this could also be set by Init functions
  // with InitByFunc
  if (variablesMap[varName] == InitFlag::NotInit) {
    variablesMap[varName] = flag;
  }
}

void DiagnosticsMatcher::NonInitializedMemberChecker::checkValueDecl(
    Expr *expr, std::unordered_map<std::string, InitFlag> &variablesMap,
    std::unordered_map<std::string, std::string> &resolverMap, InitFlag flag) {
  expr = expr->IgnoreImplicit();
  MemberExpr *memberExpr = dyn_cast_or_null<MemberExpr>(expr);
  StringRef varFromExpr;

  // And it has to be a member of 'this'.
  if (memberExpr && isa<CXXThisExpr>(memberExpr->getBase())) {
    if (!memberExpr->getMemberDecl()) {
      return;
    }
    varFromExpr = memberExpr->getMemberDecl()->getName();
    updateVarMap(variablesMap, varFromExpr, flag);
  } else {
    // maybe we should use the resolverMap since a member of |this| could
    // have been passed by reference or address
    DeclRefExpr *declExpr = dyn_cast_or_null<DeclRefExpr>(expr);
    if (declExpr) {
      if (!declExpr->getDecl()) {
        return;
      }
      varFromExpr = resolveMapVar(resolverMap, declExpr->getDecl()->getName());
      if (!varFromExpr.empty()) {
        updateVarMap(variablesMap, varFromExpr, flag);
      }
    } else {
      // if variable is dereferenced means it could have been passed by address
      UnaryOperator *unaryOp = dyn_cast_or_null<UnaryOperator>(expr);
      if (unaryOp) {
        if (unaryOp->getOpcode() != UO_Deref &&
            unaryOp->getOpcode() != UO_AddrOf) {
          return;
        }
        Expr *varExp = unaryOp->getSubExpr();
        if (!varExp) {
          return;
        }
        checkValueDecl(varExp, variablesMap, resolverMap, flag);
      }
    }
  }
}

void DiagnosticsMatcher::NonInitializedMemberChecker::runThroughDefaultFunctions(
    CallExpr *funcCall, std::unordered_map<std::string, InitFlag> &variablesMap,
    std::unordered_map<std::string, std::string> &resolverMap, InitFlag flag) {

  static struct externFuncDefinition {
    std::string funcName;
    uint8_t indexInArgList;
  } funcSchemaArray[] = {// for memset
                         {"memset", 0}};
  static uint8_t numberOfFuncs =
      sizeof(funcSchemaArray) / sizeof(externFuncDefinition);

  FunctionDecl *method = funcCall->getDirectCallee();

  // if method cannot be looked up continue
  if (!method) {
    return;
  }

  for (int i = 0; i < numberOfFuncs; i++) {
    externFuncDefinition &funcSchema = funcSchemaArray[i];
    if (funcSchema.funcName == method->getName().data()) {
      Expr *exprArg = funcCall->getArg(funcSchema.indexInArgList);
      if (!exprArg) {
        return;
      }
      checkValueDecl(exprArg, variablesMap, resolverMap, flag);
    }
  }
}

void DiagnosticsMatcher::NonInitializedMemberChecker::evaluateChildrenIfNotNull(
    Stmt *stmtExpr, std::unordered_map<std::string, InitFlag> &variablesMap,
    std::unordered_map<std::string, std::string> &resolverMap, uint8_t depth,
    InitFlag flag) {
  if (stmtExpr) {
    for (auto child : stmtExpr->children()) {
      evaluateExpression(child, variablesMap, resolverMap, depth + 1, flag);
    }
  }
}

void DiagnosticsMatcher::NonInitializedMemberChecker::evaluateExpression(
    Stmt *stmtExpr, std::unordered_map<std::string, InitFlag> &variablesMap,
    std::unordered_map<std::string, std::string> &resolverMap, uint8_t depth,
    InitFlag flag) {
  if (!stmtExpr) {
    return;
  }

  stmtExpr = stmtExpr->IgnoreImplicit();

  // Check depth and if it's equal equal with MAX_DEPTH return
  if (depth == MAX_DEPTH) {
    return;
  }

  if (BinaryOperator *binOp = dyn_cast_or_null<BinaryOperator>(stmtExpr)) {
    // If BinaryOperator check the left and right operands - left for value decl
    // right for function call
    if (binOp->getOpcode() == BO_Assign) {
      Expr *exprLeft = binOp->getLHS();
      if (!exprLeft) {
        return;
      }
      checkValueDecl(exprLeft, variablesMap, resolverMap, flag);
      Expr *exprRight = binOp->getRHS();
      if (!exprRight) {
        return;
      }
      evaluateExpression(exprRight, variablesMap, resolverMap, depth + 1, flag);
    }
  } else if (DeclStmt* declStmt = dyn_cast_or_null<DeclStmt>(stmtExpr)) {
    for (auto decl : declStmt->decls()) {
      VarDecl* varDecl = dyn_cast_or_null<VarDecl>(decl);
      if (!varDecl || !varDecl->isLocalVarDecl()) {
        continue;
      }

      Expr *initExpr = varDecl->getInit();
      if (!initExpr) {
        continue;
      }

      // If we are dealing with a reference-to-a
      if (varDecl->getType()->isReferenceType()) {
        StringRef expVarName;
        bool isFromThis = getVarNameFromExprWithThisCheck(initExpr, expVarName);

        if (expVarName.size()) {
          // if variable is member of this do not check it in resolverMap
          if (isFromThis) {
            resolverMap[varDecl->getName()] = expVarName;
          } else {
            // valid variable name - check to see if it's present in resolverMap
            auto varFromMap = resolverMap.find(expVarName);
            if (varFromMap != resolverMap.end()) {
              resolverMap[varDecl->getName()] = varFromMap->second;
            }
          }
        }
      }

      evaluateChildrenIfNotNull(initExpr, variablesMap, resolverMap, depth, flag);
    }
  } else if (IfStmt *ifStmt = dyn_cast_or_null<IfStmt>(stmtExpr)) {
    // If this is an if statement go through then and else statements,
    // if else statement is not present just skip it

    // But first we traverse the init and cond statements
    Stmt *initStmt = ifStmt->getInit();
    Stmt *condStmt = ifStmt->getCond();
    evaluateChildrenIfNotNull(initStmt, variablesMap, resolverMap, depth, flag);
    evaluateChildrenIfNotNull(condStmt, variablesMap, resolverMap, depth, flag);

    Stmt *thenStmt = ifStmt->getThen();
    Stmt *elseStmt = ifStmt->getElse();
    if (thenStmt && elseStmt) {
      std::unordered_map<std::string, InitFlag> thenMap;
      std::unordered_map<std::string, InitFlag> elseMap;

      evaluateExpression(thenStmt, thenMap, resolverMap, depth + 1, flag);
      evaluateExpression(elseStmt, elseMap, resolverMap, depth + 1, flag);

      // Loop through the thenMap and elseMap and look for the same variables
      // set to true  and add to variablesMap only the elements that are present
      // in both maps set to true
      for (auto &item : thenMap) {
        if (item.second) {
          std::unordered_map<std::string, InitFlag>::const_iterator itemElse =
              elseMap.find(item.first);
          if (itemElse != elseMap.end() && itemElse->second) {
            variablesMap[item.first] = item.second;
          }
        }
      }
    }
  } else if (ConditionalOperator *condOp = dyn_cast_or_null<ConditionalOperator>(stmtExpr)) {
    // If this is a ternary expression then go through true and false
    // expressions, if false expression is not present just skip it

    // But first we traverse the cond expression
    Expr *condExpr = condOp->getCond();
    evaluateChildrenIfNotNull(condExpr, variablesMap, resolverMap, depth, flag);

    Expr *trueExpr = condOp->getTrueExpr();
    Expr *falseExpr = condOp->getFalseExpr();
    if (trueExpr && falseExpr) {
      std::unordered_map<std::string, InitFlag> trueMap;
      std::unordered_map<std::string, InitFlag> falseMap;

      evaluateExpression(trueExpr, trueMap, resolverMap, depth + 1, flag);
      evaluateExpression(falseExpr, falseMap, resolverMap, depth + 1, flag);

      // Loop through the trueMap and falseMap and look for the same variables
      // set to true  and add to variablesMap only the elements that are present
      // in both maps set to true
      for (auto &item : trueMap) {
        if (item.second) {
          std::unordered_map<std::string, InitFlag>::const_iterator itemFalse =
              falseMap.find(item.first);
          if (itemFalse != falseMap.end() && itemFalse->second) {
            variablesMap[item.first] = item.second;
          }
        }
      }
    }
  } else if (SwitchStmt *switchStmt = dyn_cast_or_null<SwitchStmt>(stmtExpr)) {
    // If this is a switch statement go through all cases

    // But first we traverse the init and cond statements
    Stmt *initStmt = switchStmt->getInit();
    Stmt *condStmt = switchStmt->getCond();
    evaluateChildrenIfNotNull(initStmt, variablesMap, resolverMap, depth, flag);
    evaluateChildrenIfNotNull(condStmt, variablesMap, resolverMap, depth, flag);

    // We initialize the referenceMap and subStmt to the first case, which will
    // be used as a point of comparison with other cases to know which fields
    // are initialized in all cases
    SwitchCase* caseItr = switchStmt->getSwitchCaseList();
    if (!caseItr) {
      return;
    }
    Stmt* subStmt = caseItr->getSubStmt();
    if (!subStmt) {
      return;
    }
    std::unordered_map<std::string, InitFlag> referenceMap;
    evaluateExpression(subStmt, referenceMap, resolverMap, depth + 1, flag);

    while ((caseItr = caseItr->getNextSwitchCase())) {
      // We loop through each case and look at the variables initialized in each
      Stmt* subStmt = caseItr->getSubStmt();
      if (!subStmt) {
        continue;
      }
      std::unordered_map<std::string, InitFlag> caseMap = referenceMap;
      evaluateExpression(subStmt, caseMap, resolverMap, depth + 1, flag);

      // Loop through the referenceMap and consider any variable as NonInit if
      // it is NonInit in the current caseMap. This way, only variables
      // initialized in all cases will remain marked as initialized at the end
      for (auto &item : referenceMap) {
        if (item.second) {
          if (!caseMap[item.first]) {
            item.second = InitFlag::NotInit;
          }
        }
      }
    }

    // Finally we update variableMap with the variables found
    for (auto &item : referenceMap) {
      if (item.second) {
        variablesMap[item.first] = item.second;
      }
    }
  } else if (CallExpr *funcCall = dyn_cast_or_null<CallExpr>(stmtExpr)) {
    // This is a CallExpr if it has body analyze it else maybe it's a default
    // system function that in it's our table. Ex. memset
    FunctionDecl *method = funcCall->getDirectCallee();
    if (!method) {
      return;
    }
    if (method->hasBody()) {
      Stmt *stmt = method->getBody();

      // if method doesn't have statements exit
      if (!stmt) {
        return;
      }

      std::unordered_map<std::string, std::string> newResolverMap;
      bool isValid = buildResolverMap(funcCall, resolverMap, newResolverMap);
      // recursive call evaluateExpression
      if (isValid) {
        evaluateExpression(stmt, variablesMap, newResolverMap, depth + 1, flag);
      } else {
        // check to if it's member call expr
        CXXMemberCallExpr *memberFuncCall =
            dyn_cast_or_null<CXXMemberCallExpr>(stmtExpr);

        if (memberFuncCall &&
            isa<CXXThisExpr>(memberFuncCall->getImplicitObjectArgument())) {
          evaluateExpression(stmt, variablesMap, newResolverMap, depth + 1,
                             flag);
        }
      }
    } else {
      // we are dealing with some default function that are implemented
      // in StdC like memset
      runThroughDefaultFunctions(funcCall, variablesMap, resolverMap, flag);
    }
  } else {
    // This is not an interesting statement, we just traverse it and evaluate
    // its children

    evaluateChildrenIfNotNull(stmtExpr, variablesMap, resolverMap, depth, flag);
  }
}

void DiagnosticsMatcher::NonInitializedMemberChecker::run(
    const MatchFinder::MatchResult &Result) {
  DiagnosticsEngine &Diag = Result.Context->getDiagnostics();
  unsigned nonInitializedMemberID = Diag.getDiagnosticIDs()->getCustomDiagID(
      DiagnosticIDs::Error,
      "'%0' is not correctly initialized in the constructor of %1");
  std::unordered_map<std::string, InitFlag> variablesMap;
  std::unordered_map<std::string, std::string> resolverMap;
  const CXXRecordDecl *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("class");

  // maybe something strange and decl = nullptr
  if (!decl) {
    return;
  }

  // For each field, if they are builtinType and or if they are pointer add
  // to table
  for (auto field : decl->fields()) {
    // Ignore fields if MOZ_IGNORE_INITIALIZATION is present.
    if (hasCustomAnnotation(field, "moz_ignore_ctor_initialization")) {
      continue;
    }

    QualType type = field->getType();
    // This should be OK because the reference types are mandated to be
    // initialized by the language.
    if (!type->isBuiltinType() && !type->isPointerType()) {
      continue;
    }
    // add variable to the table
    variablesMap[field->getName()] = InitFlag::NotInit;
  }
  // First look for functions that are marked as being part of the
  // initialization
  // process and scan though them before looking at constructors since in this
  // way
  // we can avoid popping false-positive errors for variables that are not
  // initialized in the constructor
  for (auto method : decl->methods()) {
    Stmt *funcBody = method->getBody();

    // If it doesn't have custom annotation skip it
    if (!hasCustomAnnotation(method, "moz_is_class_init")) {
      continue;
    }

    if (!funcBody) {
      continue;
    }

    evaluateExpression(funcBody, variablesMap, resolverMap, 0,
                       InitFlag::InitByFunc);
  }
  // loop through all constructors and search for initializations or
  // attributions
  for (auto ctor : decl->ctors()) {

    // Ignore copy, move or compiler generated constructor
    if (ctor->isCopyOrMoveConstructor() || ctor->isImplicit()) {
      continue;
    }

    // This is when definition is done outside of declaration
    // loop through the redecls and get the one that is different
    // than the current ctor since it will be the definition.
    if (!ctor->isThisDeclarationADefinition()) {
      for (auto ctorFromList : ctor->redecls()) {
        CXXConstructorDecl *ctr =
            dyn_cast_or_null<CXXConstructorDecl>(ctorFromList);

        if (ctr && ctr->isThisDeclarationADefinition()) {
          ctor = ctr;
          break;
        }
      }
    }

    // If we didn't find a definition, skip this declaration.
    if (!ctor->isThisDeclarationADefinition()) {
      continue;
    }

    // If it's a delegating constructor, skip it, since we'll find the
    // non-delegating one eventually.
    if (ctor->isDelegatingConstructor()) {
      continue;
    }

    Stmt *body = ctor->getBody();
    if (!body) {
      continue;
    }

    // first look to see if it's a C11 declaration Ex. int a = 2 or a(2)
    for (auto init : ctor->inits()) {
      FieldDecl *variable = init->getMember();
      if (!variable) {
        continue;
      }
      // do not lookup variable in hash table, since we know that in
      // this context only member variables are allowed
      variablesMap[variable->getName()] = InitFlag::InitByCtor;
    }

    // Maybe it's initialized in the body.
    // We start at depth = 0
    evaluateExpression(body, variablesMap, resolverMap, 0,
                       InitFlag::InitByCtor);

    // look through the map for unmarked variables and pop error message, also
    // reset the ones that are set by the ctor
    for (auto varIt = variablesMap.begin(); varIt != variablesMap.end();
         varIt++) {
      // first check to see if the current var is not init in the init functions
      // if so we are not interested to see if it's init or not
      if (!varIt->second) {
        Diag.Report(ctor->getLocation(), nonInitializedMemberID)
            << varIt->first << ctor;
      } else {
        // Reset it only if it was init by ctor
        if (varIt->second == InitFlag::InitByCtor) {
          varIt->second = InitFlag::NotInit;
        }
      }
    }
  }
}
