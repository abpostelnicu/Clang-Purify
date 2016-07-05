#ifndef Debug_h
#define Debug_h

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
#include <cstdio>
#include <fstream>
#include <iostream>

#define DEBUG 

#ifdef DEBUG

#define Log(str)  do { cout <<"[" <<__func__<<"]"<< str << endl; } while(0)

static inline const std::string& printExpr(clang::Expr *expr) {
  static clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  static clang::PrintingPolicy Policy(LangOpts);
  static std::string TypeS;
  static llvm::raw_string_ostream s(TypeS);
  TypeS.clear();
  expr->printPretty(s, 0, Policy);
  return s.str();
}

#else
#define Log(str)  do {  } while(0)
#endif

#endif
