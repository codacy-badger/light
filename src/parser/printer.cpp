#pragma once

#include "parser/printer.hpp"

#include <stdio.h>
#include <assert.h>

void _tabs (int count) {
	for (int i = 0; i < count; i++) printf("  ");
}

void ASTPrinter::print (Ast_Statement* stm, int tabs) {
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			_tabs(tabs);
			print(static_cast<Ast_Block*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_IMPORT: {
			_tabs(tabs);
			print(static_cast<Ast_Import*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			_tabs(tabs);
			print(static_cast<Ast_Declaration*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_RETURN: {
			_tabs(tabs);
			print(static_cast<Ast_Return*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			_tabs(tabs);
			print(static_cast<Ast_Expression*>(stm), tabs);
			printf(";\n");
			break;
		}
		default: printf("-???-\n");
	}
}

void ASTPrinter::print (Ast_Block* block, int tabs) {
	printf(" {\n");
	for (auto stm: block->list) {
		print(stm, tabs + 1);
		//printf("\n");
	}
	printf("}");
}

void ASTPrinter::print (Ast_Note* note, int tabs) {
	printf("#%s", note->name);
}

void ASTPrinter::print (Ast_Declaration* decl, int tabs, bool nameOnly) {
	for (auto note : decl->notes) {
		print(note, tabs);
		printf("\n");
	}

	printf("%s : ", decl->name);
	print(decl->type, tabs, false);
	if (decl->expression) {
		if (decl->decl_flags & DECL_FLAG_CONSTANT)
			printf(" : ");
		else printf(" = ");
		print(decl->expression, tabs);
	}
	printf(";\n");
}

void ASTPrinter::print (Ast_Return* ret, int tabs) {
	printf("return ");
	if (ret->exp) print(ret->exp, tabs);
	printf(";\n");
}

void ASTPrinter::print (Ast_Import* imp, int tabs) {
	printf("import %s;", imp->filepath);
}

void ASTPrinter::print (Ast_Expression* exp, int tabs) {
	switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION: {
			print(static_cast<Ast_Type_Definition*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_FUNCTION: {
			print(static_cast<Ast_Function*>(exp), tabs);
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
			print(static_cast<Ast_Ident*>(exp), tabs, true);
			break;
		}
		case AST_EXPRESSION_LITERAL: {
			print(static_cast<Ast_Literal*>(exp), tabs);
			break;
		}
		default: printf("-???-\n");
	}
}

void ASTPrinter::print (Ast_Type_Instance* type_inst, int tabs, bool nameOnly) {
	switch (type_inst->type_inst_type) {
		case AST_TYPE_INST_UNDEFINED: printf("!UNDEFINED!");
		case AST_TYPE_INST_NAMED: {
			print(static_cast<Ast_Named_Type*>(type_inst), tabs);
			break;
		}
		case AST_TYPE_INST_POINTER: {
			print(static_cast<Ast_Pointer_Type*>(type_inst), tabs);
			break;
		}
		case AST_TYPE_INST_FUNCTION: {
			print(static_cast<Ast_Function_Type*>(type_inst), tabs);
			break;
		}
		default: printf("!UNKNOWN_TYPE!");
	}
}

void ASTPrinter::print (Ast_Named_Type* type, int tabs, bool nameOnly) {
	print(type->identifier, tabs, nameOnly);
}

void ASTPrinter::print (Ast_Pointer_Type* type, int tabs, bool nameOnly) {
	printf("*");
	print(type->base, tabs, nameOnly);
}

void ASTPrinter::print (Ast_Function_Type* type, int tabs, bool nameOnly) {
	printf("( ");
	if (type->parameters.size() > 0) {
		print(type->parameters[0], tabs, true);
		for (size_t i = 1; i < type->parameters.size(); i++) {
			printf(", ");
			print(type->parameters[i], tabs, true);
		}

	}
	printf(" ) -> ");
	print(type->return_type, tabs, true);
}

void ASTPrinter::print (Ast_Function* fn, int tabs, bool nameOnly) {
	if (!nameOnly) {
		printf("fn ");
	} else printf("[fn ");

	if (fn->name) printf("%s ", fn->name);

	if (!nameOnly) {
		printf("( ");
		if (fn->type->parameters.size() > 0) {
			print(fn->type->parameters[0], tabs, true);
			for (int i = 1; i < fn->type->parameters.size(); i++) {
				printf(", ");
				print(fn->type->parameters[i], tabs, true);
			}
		}
		printf(" )");
		if (fn->type->return_type != NULL) {
			printf(" -> ");
			print(fn->type->return_type, tabs, true);
		}

		if (fn->scope != NULL) print(fn->scope, tabs + 1);
	} else printf("]");
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
		default:						assert(false);
	}
	printf(" ");
	print(binop->rhs, tabs);
	printf(")");
}

void ASTPrinter::print (Ast_Unary* unop, int tabs) {
	printf("(");
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE_NUMBER: 	printf("-"); break;
		default:						assert(false);
	}
	printf(" ");
	print(unop->exp, tabs);
	printf(")");
}

void ASTPrinter::print (Ast_Function_Call* call, int tabs) {
	printf("(");
	print(call->fn, tabs);
	printf("( ");
	if (call->parameters.size() > 0) {
		print(call->parameters[0], tabs);
		for (int i = 1; i < call->parameters.size(); i++) {
			printf(", ");
			print(call->parameters[i], tabs);
		}
	}
	printf(" ))");
}

void ASTPrinter::print (Ast_Ident* ident, int tabs, bool nameOnly) {
	printf("%s", ident->name);
}

void ASTPrinter::print (Ast_Literal* cons, int tabs) {
	switch (cons->literal_type) {
		case AST_LITERAL_INTEGER:	printf("%lld", cons->integer_value); break;
		case AST_LITERAL_DECIMAL:	printf("%f", cons->decimal_value);  break;
		case AST_LITERAL_STRING: 	printf("\"%s\"", cons->string_value); break;
		default: 					assert(false);
	}
}
