#pragma once

#include "parser/ast.hpp"

class ASTPrinter {
public:
	static void print (Ast_Statement* stm, int tabs = 0) {
		if 		(auto obj = dynamic_cast<Ast_Variable*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Block*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Return*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Type_Definition*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Expression*>(stm)) {
			_tabs(tabs);
			print(obj, tabs);
			cout << "\n";
		} else {
			std::string msg = "Unrecognized statement?! -> ";
			msg += typeid(*stm).name();
			msg += "\n";
			_panic(msg.c_str());
		}
	}

	static void print (Ast_Variable* stm, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) {
			_tabs(tabs);
			cout << "let ";
		} else cout << "[";

		cout << stm->name;
		cout << " : ";
		if (stm->type != nullptr) print(stm->type, tabs, true);
		else cout << "[NULL]";

		if (!nameOnly) {
			if (stm->expression != nullptr) {
				cout << " = ";
				print(stm->expression, tabs);
			}

			cout << "\n";
		} else cout << "]";
	}

	static void print (Ast_Block* stm, int tabs = 0) {
		for(auto const& stm: stm->list) print(stm, tabs);
	}

	static void print (Ast_Function* stm, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) {
			_tabs(tabs);
			cout << "fn ";
		} else cout << "[fn ";

		cout << stm->name;

		if (!nameOnly) {
			cout << " ( ";
			if (stm->type->params.size() > 0) {
				print(stm->type->params[0], tabs, true);
				for (int i = 1; i < stm->type->params.size(); i++) {
					cout << ", ";
					print(stm->type->params[i], tabs, true);
				}
			}
			cout << " )";
			if (stm->type->retType != nullptr) {
				cout << " -> ";
				print(stm->type->retType, tabs, true);
			}

			cout << "\n";
			if (stm->stm != nullptr) print(stm->stm, tabs + 1);
		} else cout << "]";
	}

	static void print (Ast_Return* ret, int tabs = 0) {
		_tabs(tabs);
		cout << "return ";
		if (ret->exp == nullptr) cout << "void";
		else print(ret->exp, tabs);
		cout << endl;
	}

	static void print (Ast_Type_Definition* type, int tabs = 0, bool nameOnly = false) {
		if 		(auto obj = dynamic_cast<Ast_Primitive_Type*>(type)) 	print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<Ast_Struct_Type*>(type))    	print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<Ast_Pointer_Type*>(type))    print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<Ast_Function_Type*>(type))    		print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<Ast_Unresolved*>(type))    	print(obj, tabs);
		else {
			std::string msg = "Unrecognized type struct?! -> ";
			msg += typeid(*type).name();
			msg += "\n";
			_panic(msg.c_str());
		}
	}

	static void print (Ast_Primitive_Type* type, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) cout << "primitive type ";
		cout << type->name;
		if (!nameOnly) cout << "\n";
	}

	static void print (Ast_Struct_Type* type, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) cout << "type ";
		cout << type->name;
		if (!nameOnly) {
			cout << "\n";
			for (auto const &attr : type->attrs) print(attr, tabs + 1);
			for (auto const &mthd : type->methods) print(mthd, tabs + 1);
		}
	}

	static void print (Ast_Pointer_Type* type, int tabs = 0, bool nameOnly = false) {
		cout << "*";
		print(type->base, tabs, nameOnly);
	}

	static void print (Ast_Function_Type* type, int tabs = 0, bool nameOnly = false) {
		cout << "+FnType+" ;
	}

	static void print (Ast_Expression* exp, int tabs = 0) {
		if 		(auto obj = dynamic_cast<AST_Binary*>(exp))    	print(obj, tabs);
		else if (auto obj = dynamic_cast<AST_Unary*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Literal*>(exp))  	print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Function_Call*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Attribute*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<AST_Ref*>(exp))      	print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Deref*>(exp))    	print(obj, tabs);
		else if (auto obj = dynamic_cast<Ast_Function*>(exp)) 	print(obj, tabs, true);
		else if (auto obj = dynamic_cast<Ast_Variable*>(exp)) 	print(obj, tabs, true);
		else if (auto obj = dynamic_cast<Ast_Unresolved*>(exp)) 	print(obj, tabs);
		else {
			std::string msg = "Unrecognized expression?! -> ";
			msg += typeid(*exp).name();
			msg += "\n";
			_panic(msg.c_str());
		}
	}

	static void print (AST_Binary* binop, int tabs = 0) {
		cout << "(";
		print(binop->lhs, tabs);
		cout << " ";
		switch (binop->op) {
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

	static void print (AST_Unary* unop, int tabs = 0) {
		cout << "(";
		switch (unop->op) {
			case AST_UNARY_NEGATE: 	cout << "-"; break;
			default: 				cout << "_?_";
		}
		cout << " ";
		print(unop->exp, tabs);
		cout << ")";
	}

	static void print (Ast_Literal* cons, int tabs = 0) {
		switch (cons->type) {
			case Ast_Literal::BYTE:   cout << cons->byteValue;   break;
			case Ast_Literal::SHORT:  cout << cons->shortValue;  break;
			case Ast_Literal::INT:    cout << cons->intValue;    break;
			case Ast_Literal::LONG:   cout << cons->longValue;   break;
			case Ast_Literal::FLOAT:  cout << cons->floatValue;  break;
			case Ast_Literal::DOUBLE: cout << cons->doubleValue; break;
			case Ast_Literal::STRING: cout << "\"" << cons->stringValue << "\""; break;
			default: break;
		}
	}

	static void print (AST_Ref* ref, int tabs = 0) {
		cout << "(*";
		print(ref->memory, tabs);
		cout << ")";
	}

	static void print (Ast_Deref* deref, int tabs = 0) {
		cout << "(&";
		print(deref->memory, tabs);
		cout << ")";
	}

	static void print (Ast_Function_Call* call, int tabs = 0) {
		cout << "(";
		print(call->fn, tabs);
		cout << "( ";
		if (call->params.size() > 0) {
			print(call->params[0], tabs);
			for (int i = 1; i < call->params.size(); i++) {
				cout << ", ";
				print(call->params[i], tabs);
			}
		}
		cout << " ))";
	}

	static void print (Ast_Attribute* attr, int tabs = 0) {
		cout << "((";
		print(attr->exp, tabs);
		cout << ") ATTR " << attr->name << ")";
	}

	static void print (Ast_Unresolved* unres, int tabs = 0) {
		cout << "!_";
		cout << unres->name;
		cout << "_!";
	}
private:
	static void _tabs (int count) {
		for (int i = 0; i < count; i++) cout << "\t";
	}

	static void _panic (const char* message) {
		cout << "ERROR: " << message;
		exit(1);
	}
};
