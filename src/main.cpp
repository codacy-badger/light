#include "compiler.hpp"

int main (int argc, char** argv) {
    auto compiler = new Compiler(argc, argv);

	compiler->compile_input_files();

	delete compiler;
	return 0;
}
