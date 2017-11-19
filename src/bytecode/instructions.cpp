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

			else if (decl_ty == Light_Compiler::inst->type_def_u8)  return BYTECODE_TYPE_U32;
			else if (decl_ty == Light_Compiler::inst->type_def_u16) return BYTECODE_TYPE_U32;
	  		else if (decl_ty == Light_Compiler::inst->type_def_u32) return BYTECODE_TYPE_U32;
	  		else if (decl_ty == Light_Compiler::inst->type_def_u64) return BYTECODE_TYPE_U32;

			else if (decl_ty == Light_Compiler::inst->type_def_s8)  return BYTECODE_TYPE_S32;
			else if (decl_ty == Light_Compiler::inst->type_def_s16) return BYTECODE_TYPE_S32;
			else if (decl_ty == Light_Compiler::inst->type_def_s32) return BYTECODE_TYPE_S32;
			else if (decl_ty == Light_Compiler::inst->type_def_s64) return BYTECODE_TYPE_S32;

			else if (decl_ty == Light_Compiler::inst->type_def_s32) return BYTECODE_TYPE_F32;
			else if (decl_ty == Light_Compiler::inst->type_def_s64) return BYTECODE_TYPE_F64;

	  		else if (decl_ty == Light_Compiler::inst->type_def_string) return BYTECODE_TYPE_POINTER;
		}
	}
	return BYTECODE_TYPE_VOID;
}

uint8_t bytecode_get_type (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
		return bytecode_get_type(static_cast<Ast_Type_Definition*>(exp));
	} else return BYTECODE_TYPE_VOID;
}

size_t bytecode_get_size (uint8_t bytecode_type) {
	switch (bytecode_type) {
		case BYTECODE_TYPE_U8:
		case BYTECODE_TYPE_S8:  return 1;
		case BYTECODE_TYPE_U16:
		case BYTECODE_TYPE_S16: return 2;
		case BYTECODE_TYPE_U32:
		case BYTECODE_TYPE_S32:
		case BYTECODE_TYPE_F32: return 4;
		case BYTECODE_TYPE_U64:
		case BYTECODE_TYPE_S64:
		case BYTECODE_TYPE_F64: return 8;
		case BYTECODE_TYPE_POINTER:
		case BYTECODE_TYPE_STRING: return AST_PTR_SIZE;
		default: return 0;
	}
}
