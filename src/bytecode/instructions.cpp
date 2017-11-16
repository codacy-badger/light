#include "bytecode/instructions.hpp"

#include <assert.h>

#include "compiler.hpp"

uint8_t bytecode_get_type (Ast_Type_Definition* decl_ty) {
	switch (decl_ty->typedef_type) {
		default: assert(false);
		case AST_TYPEDEF_POINTER:
		case AST_TYPEDEF_FUNCTION: return BYTECODE_TYPE_POINTER;
		case AST_TYPEDEF_STRUCT: {
				 if (decl_ty == Light_Compiler::inst->type_def_void) return BYTECODE_TYPE_VOID;
			else if (decl_ty == Light_Compiler::inst->type_def_s32) return BYTECODE_TYPE_S32;
			// TODO: add more native types as they're added to the language
		}
	}
	return BYTECODE_TYPE_VOID;
}

uint8_t bytecode_get_type (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
		return bytecode_get_type(static_cast<Ast_Type_Definition*>(exp));
	} else return BYTECODE_TYPE_VOID;
}
