@echo off

cd examples
..\bin\light.exe examples\tests\import\main.li
..\bin\light.exe examples\tests\calls\main.li
..\bin\light.exe examples\tests\slice\main.li
..\bin\light.exe examples\tests\multi\main.li
..\bin\light.exe examples\tests\defer\main.li
cd ..
