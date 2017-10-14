@echo off

pushd build

cl /nologo /Od /c /MD /Zi /GR -I"../src" -I"../include" %LLVMCLFlags% ^
	../src/*.cpp ../src/lexer/*.cpp ../src/parser/*.cpp ../src/parser/pipe/*.cpp ^
	 ../src/back/llvm/*.cpp

if errorlevel 1 (
	echo Stoping due to compilation error/s
	popd
	exit /B
)

link /nologo /ENTRY:mainCRTStartup /OUT:"..\bin\light.exe" /DEBUG %LLVMLibs% ^
 	psapi.lib shell32.lib ole32.lib uuid.lib *.obj

popd
