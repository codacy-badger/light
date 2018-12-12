#include "ast/printer.hpp"

void _tabs (int count) {
	for (int i = 0; i < count; i++) printf("    ");
}

void ASTPrinter::print (Ast_Statement* stm, int tabs) {
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			print(static_cast<Ast_Scope*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			_tabs(tabs);
			print(static_cast<Ast_Declaration*>(stm), tabs);
			printf(";\n");
			break;
		}
		case AST_STATEMENT_RETURN: {
			print(static_cast<Ast_Return*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			_tabs(tabs);
			print(static_cast<Ast_Expression*>(stm), tabs);
			printf(";\n");
			break;
		}
		default: printf("-STM?-");
	}
}

void ASTPrinter::print (Ast_Scope* block, int tabs) {
	_tabs(tabs);
	printf("{\n");
	for (auto stm: block->statements) {
		print(stm, tabs + 1);
	}
	printf("}");
}

void ASTPrinter::print (Ast_Note* note, int tabs) {
	_tabs(tabs);
	printf("#%s", note->name);
}

void ASTPrinter::print (Ast_Declaration* decl, int tabs) {
	for (auto note : decl->notes) {
		print(note, tabs);
		printf(", ");
	}

	printf("%s : ", decl->name);

	if (decl->type) print(decl->type, tabs, true);

	if (decl->expression) {
		if (decl->is_constant()) printf(" : ");
		else printf(" = ");

		print(decl->expression, tabs);
	}
}

void ASTPrinter::print (Ast_Return* ret, int tabs) {
	_tabs(tabs);
	printf("return ");
	if (ret->expression) print(ret->expression, tabs);
	printf(";\n");
}

void ASTPrinter::print (Ast_Expression* exp, int tabs, bool name_only) {
	switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_INSTANCE: {
			print(static_cast<Ast_Type_Instance*>(exp), tabs, name_only);
			break;
		}
		case AST_EXPRESSION_FUNCTION: {
			print(static_cast<Ast_Function*>(exp), tabs, name_only);
			break;
		}
		case AST_EXPRESSION_BINARY: {
			print(static_cast<Ast_Binary*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_UNARY: {
			print(static_cast<Ast_Unary*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_CALL: {
			print(static_cast<Ast_Function_Call*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_IDENT: {
			print(static_cast<Ast_Ident*>(exp));
			break;
		}
		case AST_EXPRESSION_LITERAL: {
			print(static_cast<Ast_Literal*>(exp));
			break;
		}
		default: printf("-EXP?-");
	}
}

void ASTPrinter::print (Ast_Type_Instance* tydef, int tabs, bool name_only) {
	switch (tydef->typedef_type) {
		case AST_TYPEDEF_FUNCTION: {
			print(static_cast<Ast_Function_Type*>(tydef), tabs);
			break;
		}
		case AST_TYPEDEF_STRUCT: {
			print(static_cast<Ast_Struct_Type*>(tydef), tabs, name_only);
			break;
		}
		case AST_TYPEDEF_POINTER: {
			print(static_cast<Ast_Pointer_Type*>(tydef), tabs);
			break;
		}
		case AST_TYPEDEF_ARRAY: {
			print(static_cast<Ast_Array_Type*>(tydef), tabs);
			break;
		}
		default: printf("(-TYPE?-)");
	}
}

void ASTPrinter::print (Ast_Struct_Type* type, int tabs, bool name_only) {
	if (type->name) printf("%s", type->name);
	if (!name_only) {
		if (type->attributes.size() > 0) {
			printf(" {\n");
			for (auto attr : type->attributes) {
				_tabs(tabs + 1);
				print(attr, tabs + 1);
				printf(";\n");
			}
			_tabs(tabs);
			printf("}");
		}
	}
}

void ASTPrinter::print (Ast_Function_Type* type, int tabs) {
	printf("fn ( ");
	if (type->arg_decls.size() > 0) {
		print(type->arg_decls[0], tabs);
		for (size_t i = 1; i < type->arg_decls.size(); i++) {
			printf(", ");
			print(type->arg_decls[i], tabs);
		}

	}
	printf(") -> ");
	print(type->ret_type, tabs, true);
}

void ASTPrinter::print (Ast_Pointer_Type* type, int tabs) {
	printf("*");
	print(type->base, tabs, true);
}

void ASTPrinter::print (Ast_Array_Type* type, int tabs) {
	printf("[%zd]", type->length_as_number);
	print(type->base, tabs, true);
}

void ASTPrinter::print (Ast_Function* fn, int tabs, bool name_only) {
	if (fn->foreign_module_name) {
		printf("%s!%s ", fn->name, fn->foreign_module_name);
	} else if (fn->name) printf("%s ", fn->name);
	if (!name_only && fn->scope) {
		print(fn->scope, tabs);
	}
}

void ASTPrinter::print (Ast_Binary* binop, int tabs) {
	printf("(");
	print(binop->lhs, tabs);
	printf(" ");
	switch (binop->binary_op) {
		case AST_BINARY_ASSIGN: 		printf("="); break;
		case AST_BINARY_ATTRIBUTE: 		printf("."); break;
		case AST_BINARY_SUBSCRIPT: 		printf("["); break;
		case AST_BINARY_ADD: 			printf("+"); break;
		case AST_BINARY_SUB: 			printf("-"); break;
		case AST_BINARY_MUL: 			printf("*"); break;
		case AST_BINARY_DIV: 			printf("/"); break;
		default:						abort();
	}
	printf(" ");
	print(binop->rhs, tabs);
	printf(")");
}

void ASTPrinter::print (Ast_Unary* unop, int tabs) {
	printf("(");
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: 	printf("-"); break;
		default:				abort();
	}
	printf(" ");
	print(unop->exp, tabs);
	printf(")");
}

void ASTPrinter::print (Ast_Function_Call* call, int tabs) {
	print(call->func, tabs, true);
	printf("( ");
	if (call->arguments.size() > 0) {
		print(call->arguments[0], tabs);
		for (int i = 1; i < call->arguments.size(); i++) {
			printf(", ");
			print(call->arguments[i], tabs);
		}
	}
	printf(" )");
}

void ASTPrinter::print (Ast_Ident* ident) {
	printf("%s", ident->name);
}

void ASTPrinter::print (Ast_Literal* cons) {
	switch (cons->literal_type) {
		case AST_LITERAL_SIGNED_INT:	printf("%lld", cons->int_value); break;
		case AST_LITERAL_UNSIGNED_INT:	printf("%lld", cons->uint_value); break;
		case AST_LITERAL_DECIMAL:		printf("%lf", cons->decimal_value); break;
		case AST_LITERAL_STRING: 		printf("\"%s\"", cons->string_value); break;
		default: 						abort();
	}
}
