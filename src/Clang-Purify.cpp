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

#include "Config.h"
#include "ClangParser.h"

#include "ChecksIncludes.inc"

const char* MOZ_THIRD_PARTY_PATHS[] = {};
const uint32_t MOZ_THIRD_PARTY_PATHS_COUNT = 0;

int main(int argc, const char **argv) {  
  ClangParser parser;
  const char **arguments = nullptr;
  int argCount = 0;
  MatchFinder finder;

  #define CHECK(cls, name) cls cls ## _(name); \
                           cls ## _.registerMatchers(&finder);
  #include "Checks.inc"
  #undef CHECK
#if USE_TEST_MODE
  arguments = (const char**)CppTestsList;
  argCount = sizeof(CppTestsList) / sizeof(char*);
#else
  if (argc < 2 )
    return 1;
  
  // substract 1 since we don't want to have the exacutable name
  argCount = argc - 1;
  // add 1 since argv[0] is the programs name
  arguments = argv + 1;
#endif

  for (int i =0; i < argCount; i++) {
    parser.parseAST(arguments[i], finder);
  }
  
  return 0;
}
