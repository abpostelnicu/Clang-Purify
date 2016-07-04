//
//  Config.h
//  Clang-Purify
//
//  Created by Andi-Bogdan Postelnicu on 04/07/16.
//
//

#ifndef Config_h
#define Config_h

#define USE_TEST_MODE 0 // run the test cases


#if USE_TEST_MODE

// List of test files included in test directory
const char * const CppTestsList[] = {
  TEST_FILES_PATH "/TestOne.cpp",
};

#endif

#endif /* Config_h */
