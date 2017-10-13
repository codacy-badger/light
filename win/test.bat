@echo off

bin\light -o bin\test.exe examples\math.li
echo.
bin\test

bin\light -o bin\test.exe examples\func.li
echo.
bin\test

bin\light -o bin\test.exe examples\struct.li
echo.
bin\test
