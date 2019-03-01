@echo off

call "os\win32\setup.bat"

if not exist build mkdir build
if not exist bin mkdir bin

pushd bin

set "SOURCES="
for /R "../src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"
for /R "../os/win32/src" %%i in (*.cpp) do @call set SOURCES=%%SOURCES%% "%%i"

REM to dissable all assertions remove the "/DCUSTOM_DEBUG" flag
set FLAGS=/nologo /Od /c /MD /MP /Zi /W4 /WX /EHsc /DCUSTOM_DEBUG
cl %FLAGS% -I"../include" -I"../dyncall" %SOURCES%

link /nologo /INCREMENTAL /ENTRY:mainCRTStartup /subsystem:console ^
    /OUT:"..\bin\light.exe" /DEBUG /NODEFAULTLIB:LIBCMT *.obj ^
    ..\dyncall\dyncall\libdyncall_s.lib user32.lib

popd
