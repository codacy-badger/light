#pragma once

#include "pipeline/pipe.hpp"

template<typename T>
T binary_fold (Ast_Binary_Type binary_op, T a, T b) {
	switch (binary_op) {
		case AST_BINARY_LOGICAL_AND: 	return a && b;
		case AST_BINARY_LOGICAL_OR: 	return a || b;

		case AST_BINARY_ADD: 			return a + b;
		case AST_BINARY_SUB: 			return a - b;
		case AST_BINARY_MUL: 			return a * b;
		case AST_BINARY_DIV: 			return a / b;

		case AST_BINARY_EQ: 			return a == b;
		case AST_BINARY_NEQ: 			return a != b;
		case AST_BINARY_LT: 			return a < b;
		case AST_BINARY_LTE: 			return a <= b;
		case AST_BINARY_GT: 			return a > b;
		case AST_BINARY_GTE: 			return a >= b;
		default: return 0;
	}
}

template<typename T>
T binary_fold_logic (Ast_Binary_Type binary_op, T a, T b) {
	switch (binary_op) {
		case AST_BINARY_BITWISE_AND: 			return a & b;
		case AST_BINARY_BITWISE_OR: 			return a | b;
		case AST_BINARY_BITWISE_XOR: 			return a ^ b;
		case AST_BINARY_BITWISE_RIGHT_SHIFT: 	return a >> b;
		case AST_BINARY_BITWISE_LEFT_SHIFT: 	return a << b;
		case AST_BINARY_REM: 					return a % b;
		default: 								return binary_fold(binary_op, a, b);
	}
}

template<typename T>
T unary_fold (Ast_Unary_Type unary_op, T a) {
	switch (unary_op) {
		case AST_UNARY_NEGATE: 	return -a;
		case AST_UNARY_NOT: 	return !a;
		default: return 0;
	}
}

Ast_Type_Instance* unary_type (Ast_Unary_Type unary_op, Ast_Expression* exp) {
	switch (unary_op) {
		case AST_UNARY_NEGATE: {
				 if (exp->inferred_type == Types::type_def_s8) 	return Types::type_def_u8;
			else if (exp->inferred_type == Types::type_def_s16) return Types::type_def_u8;
			else if (exp->inferred_type == Types::type_def_s32) return Types::type_def_u16;
			else if (exp->inferred_type == Types::type_def_s64) return Types::type_def_u32;
			else if (exp->inferred_type == Types::type_def_u8) 	return Types::type_def_s16;
			else if (exp->inferred_type == Types::type_def_u16) return Types::type_def_s32;
			else if (exp->inferred_type == Types::type_def_u32) return Types::type_def_s64;
			else if (exp->inferred_type == Types::type_def_u64) {
				WARN(exp, "negating a u64 integer may not give the right value");
				return Types::type_def_s64;
			} else return exp->inferred_type;
		}
		case AST_UNARY_NOT: return Types::type_def_bool;
		default: abort();
	}
}

struct Constant_Folding : Pipe {
	PIPE_NAME(Constant_Folding)

	void handle (Ast_Cast** cast_ptr) {
		auto cast = (*cast_ptr);

		Pipe::handle(cast_ptr);

		if (cast->value->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(cast->value);

			// @Incomplete decimal conversions won't work...
			lit->inferred_type = static_cast<Ast_Type_Instance*>(cast->cast_to);

			delete *cast_ptr;
			*cast_ptr = reinterpret_cast<Ast_Cast*>(lit);
		}
	}

	void handle (Ast_Unary** unary_ptr) {
		auto unary = (*unary_ptr);

		Pipe::handle(unary_ptr);

		if (unary->exp->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = reinterpret_cast<Ast_Literal*>(unary->exp);
			lit->inferred_type = unary_type(unary->unary_op, lit);

			switch (lit->literal_type) {
				case AST_LITERAL_UNSIGNED_INT: {
					lit->int_value = unary_fold(unary->unary_op, lit->int_value);
					if (unary->unary_op == AST_UNARY_NEGATE) {
						lit->literal_type = AST_LITERAL_SIGNED_INT;
					}
					break;
				}
				case AST_LITERAL_SIGNED_INT: {
					lit->int_value = unary_fold(unary->unary_op, lit->int_value);
					break;
				}
				case AST_LITERAL_DECIMAL: {
					lit->decimal_value = unary_fold(unary->unary_op, lit->decimal_value);
					break;
				}
				case AST_LITERAL_STRING: {
					report_warning(&unary->location, "String literal folding not supported yet!");
					break;
				}
			}
			delete *unary_ptr;
			*unary_ptr = reinterpret_cast<Ast_Unary*>(lit);
		}
	}

	void handle (Ast_Binary** binary_ptr) {
		auto binary = (*binary_ptr);

		Pipe::handle(binary_ptr);

		if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
			auto ident = static_cast<Ast_Ident*>(binary->rhs);
			if (binary->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
				auto arr_type = static_cast<Ast_Array_Type*>(binary->lhs->inferred_type);
				if (strcmp(ident->name, "length") == 0) {
					auto lit = ast_make_literal(arr_type->get_length());
					lit->inferred_type = Types::type_def_u64;
					lit->location = binary->location;

					delete *binary_ptr;
					(*binary_ptr) = reinterpret_cast<Ast_Binary*>(lit);
				} else if (strcmp(ident->name, "data") == 0) {
					auto array_ref = ast_make_unary(AST_UNARY_REFERENCE, binary->lhs);
					array_ref->inferred_type = Compiler::instance->types->get_pointer_type(arr_type->base);
					array_ref->location = binary->location;

					delete *binary_ptr;
					(*binary_ptr) = reinterpret_cast<Ast_Binary*>(array_ref);
					Pipe::handle(reinterpret_cast<Ast_Unary**>(binary_ptr));
				}
			}
		}
	}
};
