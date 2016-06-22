#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Support/Host.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <ostream>

#include "NonInitializedMemberChecker.h"
#include "AstCustomMatchers.h"
#include "ClangParser.h"
#include "Debug.h"


int main(int argc, const char **argv) {
  DiagnosticsMatcher::NonInitializedMemberChecker nonInitializedMemberChecker;
  if (argc < 2 )
    return 1;

  MatchFinder finder;
  finder.addMatcher(recordDecl(isInterestingForUninitializedInCtor())
                                .bind("class"), &nonInitializedMemberChecker);

  ClangParser parser;
  parser.parseAST(argv[1], finder);
  return 0;
}