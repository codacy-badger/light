cl /nologo /c /MDd /Zi %LLVMCLFlags% main.cpp /Fo".\build\light.obj" /Fd".\build\light.pdb"

link /nologo /ENTRY:mainCRTStartup /OUT:.\bin\light.exe /DEBUG ^
    %LLVMLinkLibs% %LLVMLinkSystemLibs%  .\build\light.obj
