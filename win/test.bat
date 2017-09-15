@echo off

bin\light -o test\test.obj

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\test.exe" build\win_std.obj test\test.obj ^
	 kernel32.lib

test\test
