#pragma once

#include "parser/pipes.hpp"
#include "parser/printer.hpp"

struct PrintPipe : Pipe {
	void onDeclaration (Ast_Declaration* decl);
};
