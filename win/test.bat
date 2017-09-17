@echo off

bin\light -o test\test.obj

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\hello.exe" build\win_std.obj test\hello.obj ^
	 kernel32.lib

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\ifelse.exe" build\win_std.obj test\ifelse.obj ^
 	 kernel32.lib

echo.
echo --- TESTS ---
test\hello
test\ifelse
