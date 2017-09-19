@echo off

SET CurrentDir=%CD%

if "%VS150COMNTOOLS%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Path to all the LLVM .h files, the folder should contains a "llvm" folder
SET LLVMIncludes=".\llvm\include"

REM Path to all the LLVM Lib files
SET LLVMLibs=".\llvm\Release\lib"

REM Flags to compile the program using CL
REM This flags have been generated by "llvm-config" command, but the include path was wrong
SET LLVMCLFlags=-I%LLVMIncludes% /DWIN32 /D_WINDOWS   /Zc:inline /Zc:strictStrings /Oi /Zc:rvalueCast /W4 -wd4141 -wd4146 -wd4180 -wd4244 -wd4258 -wd4267 -wd4291 -wd4345 -wd4351 -wd4355 -wd4456 -wd4457 -wd4458 -wd4459 -wd4503 -wd4624 -wd4722 -wd4800 -wd4100 -wd4127 -wd4512 -wd4505 -wd4610 -wd4510 -wd4702 -wd4245 -wd4706 -wd4310 -wd4701 -wd4703 -wd4389 -wd4611 -wd4805 -wd4204 -wd4577 -wd4091 -wd4592 -wd4319 -wd4324 -w14062 -we4238   /EHsc -DLLVM_BUILD_GLOBAL_ISEL -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

REM System libraries that LLVM uses to link, also generated with llvm-config
SET LLVMLinkSystemLibs=psapi.lib shell32.lib ole32.lib uuid.lib

%CurrentDir:~0,2%
cd %CurrentDir%
SET "CurrentDir="
