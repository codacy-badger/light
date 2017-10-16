@echo off

REM Current LLVM hash --> 65bdf0ae4a87e6992c24f06e2612909952468710

if exist %~dp0..\llvm\Release\lib\LLVMCore.lib (
	if not "%1"=="force" (
		echo LIB files already compiled!
		echo Use parameter "force" to recompile
		exit /B
	)
)

git submodule update --init --recursive

cd llvm

cmake -DLLVM_INCLUDE_TOOLS=OFF -DLLVM_BUILD_TOOLS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_BUILD_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_BUILD_TESTS=OFF ^
	-DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release --build . -Thost=x64

cmake --build . --target all_build --config Release

cd ..
