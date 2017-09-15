@echo off

bin\light

link /ENTRY:mainCRTStartup /SUBSYSTEM:CONSOLE /DEFAULTLIB:kernel32.lib /OUT:".\test\test.exe" build\win_std.obj test\output.obj

test\test
