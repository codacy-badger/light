#include "platform.hpp"
#include "compiler.hpp"
#include "report.hpp"

int main (int argc, char** argv) {
    auto compiler = Compiler::create(argc, argv);

	compiler->run();

	Compiler::destroy();

	return 0;
}
