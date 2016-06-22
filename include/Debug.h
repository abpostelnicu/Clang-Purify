
#include <cstdio>
#include <fstream>
#include <iostream>

#ifdef DEBUG

#define Log(str)  do { cout <<"[" <<__func__<<"]"<< str << endl; } while(0)

const std::string& printExpr(Expr *expr) {
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
