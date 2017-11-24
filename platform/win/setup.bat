@echo off

SET CurrentDir=%CD%

if "%VS150COMNTOOLS%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

%CurrentDir:~0,2%
cd %CurrentDir%
SET "CurrentDir="

git submodule update --init --recursive
