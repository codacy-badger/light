#pragma once

#include "parser/pipes.hpp"

#include <map>
#include <vector>
#include <assert.h>

using namespace std;

struct Unique_Types : Pipe {
	PIPE_NAME(Unique_Types)

	void handle (Ast_Struct_Type** _struct) {
		Pipe::handle(_struct);

		if (!(*_struct)->is_slice) {
			g_compiler->types->struct_types[(*_struct)->name] = (*_struct);
		}
	}

	void handle (Ast_Pointer_Type** ptr_type) {
		Pipe::handle(ptr_type);

	    g_compiler->types->compute_type_name_if_needed(*ptr_type);
		assert((*ptr_type)->name);
	}

	void handle (Ast_Array_Type** arr_type) {
		Pipe::handle(arr_type);

	    g_compiler->types->compute_type_name_if_needed(*arr_type);
		assert((*arr_type)->name);
	}

	void handle (Ast_Slice_Type** slice) {
		Pipe::handle(slice);

	   	g_compiler->types->compute_type_name_if_needed(*slice);
		assert((*slice)->name);
	}

	void handle (Ast_Function_Type** func_type) {
		Pipe::handle(func_type);

	    g_compiler->types->compute_type_name_if_needed(*func_type);
		assert((*func_type)->name);
	}
};
