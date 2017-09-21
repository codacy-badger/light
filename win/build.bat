@echo off

cl /nologo /c /MD /Zi /GR -I"." %LLVMCLFlags% main.cpp /Fo".\build\light.obj" /Fd".\build\light.pdb"

setlocal disableDelayedExpansion
set "LLVMLinkLibs="
for /r %%i in (%LLVMLibs%\*.lib) do @call set LLVMLinkLibs=%%LLVMLinkLibs%% "%%i"

link /nologo /ENTRY:mainCRTStartup /OUT:.\bin\light.exe /DEBUG %LLVMLinkLibs% %LLVMLinkSystemLibs%  .\build\light.obj
set "LLVMLinkLibs="
