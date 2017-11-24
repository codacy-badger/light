@echo off

pushd dyncall

call .\configure /target-x64

pushd dyncall
nmake /nologo /S /C /f Nmakefile >null
popd

popd
