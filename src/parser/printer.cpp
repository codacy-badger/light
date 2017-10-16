#pragma once

#include "parser/printer.hpp"

void _tabs (int count) {
	for (int i = 0; i < count; i++) cout << "\t";
}

void ASTPrinter::print (Ast_Statement* stm, int tabs) {
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			print(static_cast<Ast_Block*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			print(static_cast<Ast_Declaration*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_RETURN: {
			print(static_cast<Ast_Return*>(stm), tabs);
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			print(static_cast<Ast_Expression*>(stm), tabs);
			break;
		}
		default: cout << "-???-\n";
	}
}

void ASTPrinter::print (Ast_Block* block, int tabs) {
	for (auto stm: block->list) print(stm, tabs);
}

void ASTPrinter::print (Ast_Declaration* decl, int tabs, bool nameOnly) {
	_tabs(tabs);
	printf("DECL!\n");
}

void ASTPrinter::print (Ast_Return* ret, int tabs) {
	_tabs(tabs);
	cout << "return ";
	if (ret->exp == NULL) cout << "void";
	else print(ret->exp, tabs);
	cout << endl;
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
			print(static_cast<AST_Binary*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_UNARY: {
			print(static_cast<AST_Unary*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_CALL: {
			print(static_cast<Ast_Function_Call*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_IDENT: {
			print(static_cast<Ast_Ident*>(exp), tabs);
			break;
		}
		case AST_EXPRESSION_LITERAL: {
			print(static_cast<Ast_Literal*>(exp), tabs);
			break;
		}
		default: "-???-\n";
	}
}

void ASTPrinter::print (Ast_Type_Instance* type_inst, int tabs, bool nameOnly) {
	cout << "-???-\n";
}

void ASTPrinter::print (Ast_Struct_Type* type, int tabs, bool nameOnly) {
	cout << "-???-\n";
}

void ASTPrinter::print (Ast_Pointer_Type* type, int tabs, bool nameOnly) {
	cout << "*";
	print(type->base, tabs, nameOnly);
}

void ASTPrinter::print (Ast_Function_Type* type, int tabs, bool nameOnly) {
	cout << "+FnType+" ;
}

void ASTPrinter::print (Ast_Function* fn, int tabs, bool nameOnly) {
	if (!nameOnly) {
		_tabs(tabs);
		cout << "fn ";
	} else cout << "[fn ";

	cout << fn->name;

	if (!nameOnly) {
		cout << " ( ";
		if (fn->type->parameters.size() > 0) {
			print(fn->type->parameters[0], tabs, true);
			for (int i = 1; i < fn->type->parameters.size(); i++) {
				cout << ", ";
				print(fn->type->parameters[i], tabs, true);
			}
		}
		cout << " )";
		if (fn->type->return_type != NULL) {
			cout << " -> ";
			print(fn->type->return_type, tabs, true);
		}

		cout << "\n";
		if (fn->scope != NULL) print(fn->scope, tabs + 1);
	} else cout << "]";
}

void ASTPrinter::print (AST_Binary* binop, int tabs) {
	cout << "(";
	print(binop->lhs, tabs);
	cout << " ";
	switch (binop->binary_op) {
		case AST_BINARY_ASSIGN: 	cout << "="; break;
		case AST_BINARY_ATTRIBUTE: 	cout << "."; break;
		case AST_BINARY_ADD: 		cout << "+"; break;
		case AST_BINARY_SUB: 		cout << "-"; break;
		case AST_BINARY_MUL: 		cout << "*"; break;
		case AST_BINARY_DIV: 		cout << "/"; break;
		default: 					cout << "_?_";
	}
	cout << " ";
	print(binop->rhs, tabs);
	cout << ")";
}

void ASTPrinter::print (AST_Unary* unop, int tabs) {
	cout << "(";
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE_NUMBER: 	cout << "-"; break;
		default: 				cout << "_?_";
	}
	cout << " ";
	print(unop->exp, tabs);
	cout << ")";
}

void ASTPrinter::print (Ast_Function_Call* call, int tabs) {
	cout << "(";
	print(call->fn, tabs);
	cout << "( ";
	if (call->parameters.size() > 0) {
		print(call->parameters[0], tabs);
		for (int i = 1; i < call->parameters.size(); i++) {
			cout << ", ";
			print(call->parameters[i], tabs);
		}
	}
	cout << " ))";
}

void ASTPrinter::print (Ast_Ident* ident, int tabs, bool nameOnly) {
	cout << "!IDENT!";
}

void ASTPrinter::print (Ast_Literal* cons, int tabs) {
	switch (cons->literal_type) {
		case AST_LITERAL_INTEGER:    cout << cons->integer_value;    break;
		case AST_LITERAL_DECIMAL:  cout << cons->decimal_value;  break;
		case AST_LITERAL_STRING: cout << "\"" << cons->string_value << "\""; break;
		default: break;
	}
}
