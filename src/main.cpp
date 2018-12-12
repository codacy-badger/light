#include "compiler.hpp"

int main (int argc, char** argv) {
    auto compiler = Compiler::create(argc, argv);

	compiler->run();

	Compiler::destroy();

	return 0;
}
