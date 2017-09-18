cl /nologo /c /MDd /Zi win\std\sys.c
cl /nologo /c /MDd /Zi win\std\print.c

lib /nologo /OUT:".\build\std_li.lib" sys.obj print.obj

del *.obj *.pdb
