#pragma once

#include "parser/pipes.hpp"

struct Compile_Constants : Pipe {

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);
		
		if (strcmp(ident->name, "__FILE__") == 0) {
			auto file_literal = ast_make_literal(ident->location.filename);
			file_literal->location = (*ident_ptr)->location;
			delete *ident_ptr;
			(*ident_ptr) = reinterpret_cast<Ast_Ident*>(file_literal);
		} else if (strcmp(ident->name, "__LINE__") == 0) {
			auto line_literal = ast_make_literal(ident->location.line);
			line_literal->location = (*ident_ptr)->location;
			delete *ident_ptr;
			(*ident_ptr) = reinterpret_cast<Ast_Ident*>(line_literal);
		}
	}
};
