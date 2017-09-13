@echo off

call clean.bat

mkdir build
pushd build

cl /nologo /c /MD /Zi %LLVMCLFlags% ../main.cpp

popd

mkdir bin
pushd bin

link /nologo /ENTRY:mainCRTStartup /OUT:light.exe /DEBUG ^
    %LLVMLinkLibs% %LLVMLinkSystemLibs%  ../build/main.obj

popd
