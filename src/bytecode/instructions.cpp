#include "bytecode/instructions.hpp"

#include "compiler.hpp"

Bytecode_Type bytecode_get_type (Ast_Type_Instance* decl_ty) {
	switch (decl_ty->typedef_type) {
		case AST_TYPEDEF_ARRAY:
		case AST_TYPEDEF_POINTER:
		case AST_TYPEDEF_FUNCTION: return BYTECODE_TYPE_POINTER;
		case AST_TYPEDEF_STRUCT: {
				 if (decl_ty == g_compiler->type_def_void) return BYTECODE_TYPE_VOID;

			else if (decl_ty == g_compiler->type_def_bool) return BYTECODE_TYPE_BOOL;
			else if (decl_ty == g_compiler->type_def_u8) return BYTECODE_TYPE_U8;
			else if (decl_ty == g_compiler->type_def_u16) return BYTECODE_TYPE_U16;
			else if (decl_ty == g_compiler->type_def_u32) return BYTECODE_TYPE_U32;
			else if (decl_ty == g_compiler->type_def_u64) return BYTECODE_TYPE_U64;

			else if (decl_ty == g_compiler->type_def_s8)  return BYTECODE_TYPE_S8;
			else if (decl_ty == g_compiler->type_def_s16) return BYTECODE_TYPE_S16;
			else if (decl_ty == g_compiler->type_def_s32) return BYTECODE_TYPE_S32;
			else if (decl_ty == g_compiler->type_def_s64) return BYTECODE_TYPE_S64;

			else if (decl_ty == g_compiler->type_def_f32) return BYTECODE_TYPE_F32;
			else if (decl_ty == g_compiler->type_def_f64) return BYTECODE_TYPE_F64;
		}
	}
	return BYTECODE_TYPE_POINTER;
}

Bytecode_Type bytecode_get_type (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
		return bytecode_get_type(static_cast<Ast_Type_Instance*>(exp));
	} else return BYTECODE_TYPE_VOID;
}

size_t bytecode_get_size (Bytecode_Type bytecode_type) {
	switch (bytecode_type) {
		case BYTECODE_TYPE_BOOL:
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
		case BYTECODE_TYPE_POINTER: return AST_POINTER_SIZE;
		default: return 0;
	}
}

bool bytecode_has_sign (Bytecode_Type bytecode_type) {
	switch (bytecode_type) {
		case BYTECODE_TYPE_S8:
		case BYTECODE_TYPE_S16:
		case BYTECODE_TYPE_S32:
		case BYTECODE_TYPE_S64:
			return true;
		default: return false;
	}
}

Bytecode_Type bytecode_unsigned_to_signed (Bytecode_Type bytecode_type) {
	switch (bytecode_type) {
		case BYTECODE_TYPE_U8: 	return BYTECODE_TYPE_S16;
		case BYTECODE_TYPE_U16: return BYTECODE_TYPE_S32;
		case BYTECODE_TYPE_U32: return BYTECODE_TYPE_S64;
		case BYTECODE_TYPE_U64: return BYTECODE_TYPE_S64;
		default: 				return bytecode_type;
	}
}
