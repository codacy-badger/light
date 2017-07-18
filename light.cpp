#include <iostream>

#include "compiler.cpp"

#define LANG_NAME "Light"
#define LANG_VER_MAJOR "0"
#define LANG_VER_MINOR "1"
#define LANG_VER_PATCH "0"
#define LANG_VER LANG_VER_MAJOR "." LANG_VER_MINOR "." LANG_VER_PATCH
#define LANG_FULLNAME LANG_NAME " v" LANG_VER

int main (int argc, char *argv[]) {
	std::cout << LANG_FULLNAME << std::endl;

	Compiler* compiler = new Compiler();
	compiler->addSource("example/demo.li");
	compiler->compile(NULL);
	delete compiler;

	return 0;
}
