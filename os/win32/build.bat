@echo off

call "os\win32\setup.bat"

if not exist build mkdir build
if not exist bin mkdir bin

pushd build

REM compile using clang-cl
clang-cl /nologo /Od /c /MD /Zi /W4 /EHsc /DCUSTOM_DEBUG ^
    -I"../include" -I"../dyncall" ^
    ..\src\*.cpp ..\os\win32\src\*.cpp

REM link using lld-link
lld-link /nologo /INCREMENTAL /ENTRY:mainCRTStartup /subsystem:console ^
    /OUT:"..\bin\light.exe" /DEBUG /NODEFAULTLIB:LIBCMT *.obj ^
    ..\dyncall\dyncall\libdyncall_s.lib user32.lib

popd
