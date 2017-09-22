@echo off

bin\light -o test\test.obj examples\struct.li

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\ifelse.exe" test\ifelse.obj ^
 	build\std_li.lib kernel32.lib

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\for.exe" test\for.obj ^
 	build\std_li.lib kernel32.lib

link /nologo /ENTRY:main /SUBSYSTEM:CONSOLE /OUT:".\test\struct.exe" test\struct.obj ^
 	build\std_li.lib kernel32.lib

echo.
echo --- TESTS ---
test\ifelse
test\for
test\struct
