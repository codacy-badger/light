@echo off

pushd build

set "SOURCES="
for /R "../src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"
cl /nologo /Od /c /MD /Zi /GR /Gm -I"../src" -I"../include" %LLVMCLFlags% ^
	%SOURCES%
 set "SOURCES="

link /nologo /ENTRY:mainCRTStartup /OUT:"..\bin\light.exe" /DEBUG %LLVMLibs% ^
 	psapi.lib shell32.lib ole32.lib uuid.lib *.obj

popd
