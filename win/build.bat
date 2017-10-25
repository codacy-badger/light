@echo off

pushd build

set "SOURCES="
for /R "../src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"
cl /nologo /Od /c /MD /Zi /Gm /EHsc -I"../include" %LLVMCLFlags% %SOURCES%
set "SOURCES="

link /nologo /INCREMENTAL /ENTRY:mainCRTStartup ^
    /OUT:"..\bin\light.exe" /DEBUG %LLVMLibs% ^
 	psapi.lib shell32.lib ole32.lib uuid.lib *.obj

popd
