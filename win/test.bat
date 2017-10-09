@echo off

bin\light -o build\test.obj examples\func.li

echo.
link /nologo /ENTRY:main /OUT:"bin\test.exe" build/test.obj kernel32.lib build\std_li.lib
bin\test
