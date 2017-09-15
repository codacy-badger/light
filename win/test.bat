@echo off

bin\light

link /ENTRY:mainCRTStartup /SUBSYSTEM:CONSOLE /OUT:".\test\test.exe" build\win_std.obj test\output.obj ^
	 kernel32.lib

test\test
