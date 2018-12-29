#include "compiler.hpp"

int main (int argc, char** argv) {
    auto compiler = new Compiler(argc, argv);

	compiler->run();

	delete compiler;
	return 0;
}
