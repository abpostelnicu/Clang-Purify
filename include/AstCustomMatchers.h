#ifndef AstCustomMatchers_h
#define AstCustomMatchers_h

#include "ClangDefines.h"
#include "llvm/Support/Path.h"
#include <memory>

namespace clang {
  namespace ast_matchers {
    AST_MATCHER(CXXRecordDecl, isNotInSystemHeader) {
      const CXXRecordDecl *decl = Node.getCanonicalDecl();

      // No system headers.
      auto &SourceManager = Finder->getASTContext().getSourceManager();
      FullSourceLoc loc(decl->getLocation(), SourceManager);

      return !loc.isInSystemHeader();
    }
    
    AST_MATCHER(CXXRecordDecl, isInterestingForUninitializedInCtor) {
      const CXXRecordDecl *decl = Node.getCanonicalDecl();
      // No system headers.
      auto &SourceManager = Finder->getASTContext().getSourceManager();
      FullSourceLoc loc(decl->getLocation(), SourceManager);
      
      if (loc.isInSystemHeader()) {
        // is in system header so skip it
        return false;
      } else {
        // check to see if it's in a package path
        // that should be ignored
        SmallString<1024> fileName = SourceManager.getFilename(loc);
        llvm::sys::fs::make_absolute(fileName);
        auto begin = llvm::sys::path::rbegin(fileName);
        auto end = llvm::sys::path::rend(fileName);
        
        for (; begin != end; ++begin) {
          if (begin->compare_lower(StringRef("skia")) == 0 ||
              begin->compare_lower(StringRef("angle")) == 0 ||
              begin->compare_lower(StringRef("harfbuzz")) == 0 ||
              begin->compare_lower(StringRef("hunspell")) == 0 ||
              begin->compare_lower(StringRef("scoped_ptr.h")) == 0 ||
              begin->compare_lower(StringRef("graphite2")) == 0) {
            return false;
          }
        }
        return true;
      }
    }
  
    AST_MATCHER(CallExpr, isAssertionWithAssignment) {
      static const std::string assertName = "AssertAssignmentTest";
      const FunctionDecl *method = Node.getDirectCallee();

      return method && method->getDeclName().isIdentifier() && method->getName() == assertName;
    }
    
    AST_MATCHER(CXXMethodDecl, isMethodDeclInPath) {
      const CXXMethodDecl *decl = Node.getCanonicalDecl();
      auto &SourceManager = Finder->getASTContext().getSourceManager();
      FullSourceLoc loc(decl->getLocation(), SourceManager);
      
      // No system headers.
      if (loc.isInSystemHeader()) {
        // is in system header so skip it
        return false;
      } else {
        // check to see if it's in a package path
        // that should be ignored
        SmallString<1024> fileName = SourceManager.getFilename(loc);
        llvm::sys::fs::make_absolute(fileName);
        auto begin = llvm::sys::path::rbegin(fileName);
        auto end = llvm::sys::path::rend(fileName);
        
        for (; begin != end; ++begin) {
          if (begin->compare_lower(StringRef("skia")) == 0 ||
              begin->compare_lower(StringRef("angle")) == 0 ||
              begin->compare_lower(StringRef("harfbuzz")) == 0 ||
              begin->compare_lower(StringRef("hunspell")) == 0 ||
              begin->compare_lower(StringRef("scoped_ptr.h")) == 0 ||
              begin->compare_lower(StringRef("graphite2")) == 0) {
            return false;
          }
        }
        return true;
      }
    }
    
    AST_MATCHER(CXXMethodDecl, isMethodSuitable) {
      const CXXMethodDecl *decl = Node.getCanonicalDecl();
      
      // check to see if it's new operator
      if (decl->getDeclName().getCXXOverloadedOperator() != OO_New) {
        return false;
      }
      
      auto &SourceManager = Finder->getASTContext().getSourceManager();
      FullSourceLoc loc(decl->getLocation(), SourceManager);
      
      // No system headers.
      if (loc.isInSystemHeader()) {
        // is in system header so skip it
        return false;
      } else {
        // check to see if it's in a package path
        // that should be ignored
        SmallString<1024> fileName = SourceManager.getFilename(loc);
        llvm::sys::fs::make_absolute(fileName);
        auto begin = llvm::sys::path::rbegin(fileName);
        auto end = llvm::sys::path::rend(fileName);
        
        for (; begin != end; ++begin) {
          if (begin->compare_lower(StringRef("skia")) == 0 ||
              begin->compare_lower(StringRef("angle")) == 0 ||
              begin->compare_lower(StringRef("harfbuzz")) == 0 ||
              begin->compare_lower(StringRef("hunspell")) == 0 ||
              begin->compare_lower(StringRef("scoped_ptr.h")) == 0 ||
              begin->compare_lower(StringRef("graphite2")) == 0) {
            return false;
          }
        }
        return true;
      }
    }
    
    AST_MATCHER(CXXRecordDecl, doesClassInherits) {
      const CXXRecordDecl *decl = Node.getCanonicalDecl();
      
      // Must have definition and should inherit other classes
      return decl && decl->hasDefinition() && decl->getNumBases();
    }
  }
}

#endif // AstCustomMatchers_h
