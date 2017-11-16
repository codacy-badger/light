@echo off

pushd dyncall

call .\configure /target-x64
nmake /f Nmakefile

popd
