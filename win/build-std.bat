@echo off

for /r %%i in (win\std\*.c) do (
	cl /nologo /c /MDd /Zi "%%i"
)

set "STD_OBJS="
for %%i in (*.obj) do @call set STD_OBJS=%%STD_OBJS%% "%%i"
echo %STD_OBJS%
lib /nologo /OUT:".\build\std_li.lib" %STD_OBJS%
set "STD_OBJS="

del *.obj *.pdb
