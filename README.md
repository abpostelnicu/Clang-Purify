# Clang-Purify
Simple implementations for various clang AST checkers for static analysis performed on C and C++ code.

List of checkers that are implemented so far:
 - uninitialised members check, with deep search in called function from CTOR;
 - side effects in assert functions - like assert(a=2);
 - restricted use of specific overloaded operator;
 
 The list will be updated as new checkers will be in place.
