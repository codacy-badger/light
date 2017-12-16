@echo off

pushd build
set "SOURCES="

for /R "../src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"
for /R "../platform/win/src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"

cl /nologo /Od /c /MD /Zi /Gm /W4 /EHsc -I"../include" -I"../dyncall" %SOURCES%

link /nologo /INCREMENTAL /ENTRY:mainCRTStartup /subsystem:console ^
    /OUT:"..\bin\light.exe" /DEBUG /NODEFAULTLIB:LIBCMT *.obj ^
    ..\dyncall\dyncall\libdyncall_s.lib user32.lib

set "SOURCES="
popd
