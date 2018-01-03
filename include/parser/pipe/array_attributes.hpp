#pragma once

#include "parser/pipes.hpp"

struct Array_Attributes : Pipe {

	void handle (Ast_Binary** binary_ptr) {
		auto binary = (*binary_ptr);

		if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
			auto ident = static_cast<Ast_Ident*>(binary->rhs);
			if (binary->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
				auto arr_type = static_cast<Ast_Array_Type*>(binary->lhs->inferred_type);
				if (strcmp(ident->name, "length") == 0) {
					auto lit = ast_make_literal(arr_type->length());
					lit->inferred_type = g_compiler->type_def_u64;
					lit->location = binary->location;

					delete *binary_ptr;
					(*binary_ptr) = reinterpret_cast<Ast_Binary*>(lit);
					return;
				} else if (strcmp(ident->name, "data") == 0) {
					auto array_ref = ast_make_unary(AST_UNARY_REFERENCE, binary->lhs);
					array_ref->inferred_type = g_compiler->types->get_or_create_pointer_type(arr_type->base);
					array_ref->location = binary->location;

					delete *binary_ptr;
					(*binary_ptr) = reinterpret_cast<Ast_Binary*>(array_ref);
					Pipe::handle(reinterpret_cast<Ast_Unary**>(binary_ptr));
					return;
				}
			}
		}

		Pipe::handle(binary_ptr);
	}
};
