@echo off

pushd build
set "SOURCES="

for /R "../src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"
for /R "../platform/win/src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"

cl /nologo /Od /c /MD /Zi /Gm /EHsc -I"../include" -I"../dyncall" %SOURCES%

link /nologo /INCREMENTAL /ENTRY:mainCRTStartup ^
    /OUT:"..\bin\light.exe" /DEBUG /NODEFAULTLIB:LIBCMT *.obj ^
    ..\dyncall\dyncall\libdyncall_s.lib

set "SOURCES="
popd