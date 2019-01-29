#include "bytecode/instructions.hpp"

#include "ast/types.hpp"

#include <assert.h>

Bytecode_Type bytecode_get_type (Ast_Type* decl_ty) {
	assert(decl_ty != NULL);
	switch (decl_ty->typedef_type) {
		case AST_TYPEDEF_ARRAY:
		case AST_TYPEDEF_POINTER:
		case AST_TYPEDEF_FUNCTION: return BYTECODE_TYPE_POINTER;
		case AST_TYPEDEF_STRUCT: {
				 if (decl_ty == Types::type_void) return BYTECODE_TYPE_VOID;

			else if (decl_ty == Types::type_bool) return BYTECODE_TYPE_BOOL;
			else if (decl_ty == Types::type_u8) return BYTECODE_TYPE_U8;
			else if (decl_ty == Types::type_u16) return BYTECODE_TYPE_U16;
			else if (decl_ty == Types::type_u32) return BYTECODE_TYPE_U32;
			else if (decl_ty == Types::type_u64) return BYTECODE_TYPE_U64;

			else if (decl_ty == Types::type_s8)  return BYTECODE_TYPE_S8;
			else if (decl_ty == Types::type_s16) return BYTECODE_TYPE_S16;
			else if (decl_ty == Types::type_s32) return BYTECODE_TYPE_S32;
			else if (decl_ty == Types::type_s64) return BYTECODE_TYPE_S64;

			else if (decl_ty == Types::type_f32) return BYTECODE_TYPE_F32;
			else if (decl_ty == Types::type_f64) return BYTECODE_TYPE_F64;
		}
	}
	return BYTECODE_TYPE_POINTER;
}

Bytecode_Type bytecode_get_type (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_TYPE) {
		return bytecode_get_type(static_cast<Ast_Type*>(exp));
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
		// @TODO @FIXME this should depend on the target arch
		case BYTECODE_TYPE_POINTER: return 8;
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

Inst_Unop get_bytecode_from_unop (Ast_Unary_Type unop) {
	switch (unop) {
		case AST_UNARY_NEGATE:  return BYTECODE_ARITHMETIC_NEGATE;
		case AST_UNARY_NOT:     return BYTECODE_LOGICAL_NEGATE;
		default: 				abort();
	}
}

Inst_Binop get_bytecode_from_binop (Ast_Binary_Type binop) {
	switch (binop) {
		case AST_BINARY_LOGICAL_AND: 			return BYTECODE_LOGICAL_AND;
		case AST_BINARY_LOGICAL_OR: 			return BYTECODE_LOGICAL_OR;

		case AST_BINARY_ADD: 					return BYTECODE_ADD;
		case AST_BINARY_SUB: 					return BYTECODE_SUB;
		case AST_BINARY_MUL: 					return BYTECODE_MUL;
		case AST_BINARY_DIV: 					return BYTECODE_DIV;
		case AST_BINARY_REM: 					return BYTECODE_REM;

		case AST_BINARY_BITWISE_AND: 			return BYTECODE_BITWISE_AND;
		case AST_BINARY_BITWISE_OR: 			return BYTECODE_BITWISE_OR;
		case AST_BINARY_BITWISE_XOR: 			return BYTECODE_BITWISE_XOR;
		case AST_BINARY_BITWISE_RIGHT_SHIFT: 	return BYTECODE_BITWISE_RIGHT_SHIFT;
		case AST_BINARY_BITWISE_LEFT_SHIFT: 	return BYTECODE_BITWISE_LEFT_SHIFT;

		case AST_BINARY_EQ:						return BYTECODE_EQ;
		case AST_BINARY_NEQ:					return BYTECODE_NEQ;
		case AST_BINARY_LT:						return BYTECODE_LT;
		case AST_BINARY_LTE:					return BYTECODE_LTE;
		case AST_BINARY_GT:						return BYTECODE_GT;
		case AST_BINARY_GTE:					return BYTECODE_GTE;

		default: 								abort();
	}
}
