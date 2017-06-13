cmake_minimum_required(VERSION 3.3)
project(Clang-Purify)

if (NOT DEFINED MOZ_ROOT)
    message(FATAL_ERROR "Variable MOZ_ROOT not set. Please pass the path to the mozilla repository with -DMOZ_ROOT=")
endif()

set(CLANG_LIB "/usr/local")
set(LLVM_PATH "${CLANG_LIB}")

SET (CMAKE_C_COMPILER "${CLANG_LIB}/bin/clang")
SET (CMAKE_CXX_COMPILER "${CLANG_LIB}/bin/clang++")

EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})


if (CLANG_VERSION_STRING VERSION_LESS 3.9)
    message(FATAL_ERROR "Insufficient clang version - need at least 3.9")
endif()


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fno-rtti  -stdlib=libc++")

add_definitions(
        -D__STDC_LIMIT_MACROS
        -D__STDC_CONSTANT_MACROS
)

include_directories(${LLVM_PATH}/include)
include_directories(include)

link_directories(${LLVM_PATH}/lib)

message("Running `./mach build pre-export` to regroup headers")
EXECUTE_PROCESS( COMMAND ./mach build pre-export WORKING_DIRECTORY ${MOZ_ROOT} OUTPUT_VARIABLE pre_export_output RESULT_VARIABLE pre_export_result )

if (NOT (pre_export_result EQUAL 0))
    message("`./mach build pre-export` failed. Maybe the configuration doesn't exist")
    message("Running `./mach configure` to generate configuration")
    EXECUTE_PROCESS( COMMAND ./mach configure WORKING_DIRECTORY ${MOZ_ROOT} OUTPUT_QUIET RESULT_VARIABLE configure_result )
    if (NOT (configure_result EQUAL 0))
        message(FATAL_ERROR "`./mach configure` failed. Aborting")
    endif()

    message("Re-running `./mach build pre-export` to regroup headers")
    EXECUTE_PROCESS( COMMAND ./mach build pre-export WORKING_DIRECTORY ${MOZ_ROOT} OUTPUT_VARIABLE pre_export_output RESULT_VARIABLE pre_export_result )
    if (NOT (pre_export_result EQUAL 0))
        message(FATAL_ERROR "`./mach build pre-export` failed. Aborting")
    endif()
endif()

string (REGEX REPLACE ".*make -C (.*) -j[0-9]* -s backend.*" "\\1" OBJ_ROOT ${pre_export_output})

include_directories("${MOZ_ROOT}/build/clang-plugin/" "${OBJ_ROOT}/dist/include/" "include/")

file(GLOB SOURCE_FILES "${MOZ_ROOT}/build/clang-plugin/*.cpp")
#file(GLOB_RECURSE TEST_FILES "${MOZ_ROOT}/build/clang-plugin/tests/*.cpp")

add_definitions( -DHAVE_NEW_ASTMATCHER_NAMES )

add_executable(Clang-Purify "src/Clang-Purify.cpp" "src/ClangParser.cpp" ${SOURCE_FILES})


# get full path for test files
get_filename_component(TEST_FILES_FULL_PATH "${MOZ_ROOT}/build/clang-plugin/tests/" ABSOLUTE)

# add a definition in project with full path 
target_compile_definitions(Clang-Purify PRIVATE TEST_FILES_PATH="${TEST_FILES_FULL_PATH}")


if(LINUX) 

target_link_libraries(Clang-Purify
        ffi
        edit
        m
        clangFrontend
        clangFrontendTool
        clangSerialization
        clangDriver
        clangCodeGen # 1
        clangParse
        clangSema
        clangStaticAnalyzerCheckers # 2
        clangStaticAnalyzerCore #3
        clangAnalysis
        clangIndex
        clangRewrite
        clangAST
        clangASTMatchers
        clangBasic
        clangEdit
        clangLex
        clangTooling
        LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
        LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
        LLVMX86AsmPrinter # MC, Support, X86Utils
        LLVMX86Info # MC, Support, Target
        LLVMX86Utils # Core, Support
        LLVMipo
        LLVMScalarOpts
        LLVMInstCombine
        LLVMTransformUtils
        LLVMAnalysis
        LLVMTarget
        LLVMOption # Support
        LLVMMCParser # MC, Support
        LLVMMC # Object, Support
        LLVMObject # BitReader, Core, Support
        LLVMBitReader # Core, Support
        LLVMCore
        LLVMSupport
        z
        tinfo
        dl
        pthread
)

endif()

if(APPLE) 

target_link_libraries(Clang-Purify
        ffi
        edit
        ncurses
        m
        clangFrontend
        clangFrontendTool
        clangSerialization
        clangDriver
        clangCodeGen # 1
        clangParse
        clangSema
        clangStaticAnalyzerCheckers # 2
        clangStaticAnalyzerCore #3
        clangAnalysis
        clangIndex
        clangRewrite
        clangAST
        clangASTMatchers
        clangBasic
        clangEdit
        clangLex
        clangTooling
        LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
        LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
        LLVMX86AsmPrinter # MC, Support, X86Utils
        LLVMX86Info # MC, Support, Target
        LLVMX86Utils # Core, Support
        LLVMipo
        LLVMScalarOpts
        LLVMInstCombine
        LLVMTransformUtils
        LLVMAnalysis
        LLVMTarget
        LLVMOption # Support
        LLVMMCParser # MC, Support
        LLVMMC # Object, Support
        LLVMObject # BitReader, Core, Support
        LLVMBitReader # Core, Support
        LLVMCore
        LLVMSupport
        LLVMProfileData
        LLVMDemangle
        z
        dl
        pthread
)

endif()

