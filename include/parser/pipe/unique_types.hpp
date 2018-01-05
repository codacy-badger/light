#pragma once

#include "parser/pipes.hpp"

#include <map>
#include <vector>

using namespace std;

struct Unique_Types : Pipe {
	PIPE_NAME(Unique_Types)

	void handle (Ast_Struct_Type** _struct) {
		Pipe::handle(_struct);
		
	    g_compiler->types->compute_type_name_if_needed(*_struct);
		g_compiler->types->struct_types[(*_struct)->name] = (*_struct);
	}

	void handle (Ast_Pointer_Type** ptr_type) {
		Pipe::handle(ptr_type);

	    g_compiler->types->compute_type_name_if_needed(*ptr_type);
	}

	void handle (Ast_Array_Type** arr_type) {
		Pipe::handle(arr_type);

	    (*arr_type) = g_compiler->types->get_unique_array_type(*arr_type);
	    g_compiler->types->compute_type_name_if_needed(*arr_type);
	}

	void handle (Ast_Function_Type** func_type) {
		Pipe::handle(func_type);

	    (*func_type) = g_compiler->types->get_unique_function_type(*func_type);
	    g_compiler->types->compute_type_name_if_needed(*func_type);
	}
};
