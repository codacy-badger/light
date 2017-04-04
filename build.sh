rm -r build/*
mkdir -p build
gcc -O2 -std=c99 test/main.c -o build/test
