#pragma once

#include "ast/ast.hpp"

class ASTPrinter {
public:
	static void print (ASTStatement* stm, int tabs = 0) {
		if 		(auto obj = dynamic_cast<ASTVariable*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTScope*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTFunction*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTReturn*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTType*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTExpression*>(stm)) {
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

	static void print (ASTVariable* stm, int tabs = 0, bool nameOnly = false) {
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

	static void print (ASTScope* stm, int tabs = 0) {
		for(auto const& stm: stm->list) print(stm, tabs);
	}

	static void print (ASTFunction* stm, int tabs = 0, bool nameOnly = false) {
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

	static void print (ASTReturn* ret, int tabs = 0) {
		_tabs(tabs);
		cout << "return ";
		if (ret->exp == nullptr) cout << "void";
		else print(ret->exp, tabs);
		cout << endl;
	}

	static void print (ASTType* type, int tabs = 0, bool nameOnly = false) {
		if 		(auto obj = dynamic_cast<ASTPrimitiveType*>(type)) 	print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<ASTStructType*>(type))    	print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<ASTPointerType*>(type))    print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<ASTFnType*>(type))    		print(obj, tabs, nameOnly);
		else if (auto obj = dynamic_cast<ASTUnresolved*>(type))    	print(obj, tabs);
		else {
			std::string msg = "Unrecognized type struct?! -> ";
			msg += typeid(*type).name();
			msg += "\n";
			_panic(msg.c_str());
		}
	}

	static void print (ASTPrimitiveType* type, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) cout << "primitive type ";
		cout << type->name;
	}

	static void print (ASTStructType* type, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) cout << "struct type ";
		cout << type->name;
		if (!nameOnly) {
			cout << "\n";
			for (auto const &attr : type->attrs) print(attr, tabs + 1);
			cout << "\n";
			for (auto const &mthd : type->methods) print(mthd, tabs + 1);
		}
	}

	static void print (ASTPointerType* type, int tabs = 0, bool nameOnly = false) {
		cout << "*";
		print(type->base, tabs, nameOnly);
	}

	static void print (ASTFnType* type, int tabs = 0, bool nameOnly = false) {
		cout << "+FnType+" ;
	}

	static void print (ASTExpression* exp, int tabs = 0) {
		if 		(auto obj = dynamic_cast<ASTBinop*>(exp))    	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTUnop*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTLiteral*>(exp))  	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTCall*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTAttr*>(exp))     	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTRef*>(exp))      	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTDeref*>(exp))    	print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTFunction*>(exp)) 	print(obj, tabs, true);
		else if (auto obj = dynamic_cast<ASTVariable*>(exp)) 	print(obj, tabs, true);
		else if (auto obj = dynamic_cast<ASTUnresolved*>(exp)) 	print(obj, tabs);
		else {
			std::string msg = "Unrecognized expression?! -> ";
			msg += typeid(*exp).name();
			msg += "\n";
			_panic(msg.c_str());
		}
	}

	static void print (ASTBinop* binop, int tabs = 0) {
		cout << "(";
		print(binop->lhs, tabs);
		cout << " " << ASTBinop::opChar[binop->op] << " ";
		print(binop->rhs, tabs);
		cout << ")";
	}

	static void print (ASTUnop* unop, int tabs = 0) {
		cout << "(" << ASTUnop::opChar[unop->op] << " ";
		print(unop->exp, tabs);
		cout << ")";
	}

	static void print (ASTLiteral* cons, int tabs = 0) {
		switch (cons->type) {
			case ASTLiteral::BYTE:   cout << cons->byteValue;   break;
			case ASTLiteral::SHORT:  cout << cons->shortValue;  break;
			case ASTLiteral::INT:    cout << cons->intValue;    break;
			case ASTLiteral::LONG:   cout << cons->longValue;   break;
			case ASTLiteral::FLOAT:  cout << cons->floatValue;  break;
			case ASTLiteral::DOUBLE: cout << cons->doubleValue; break;
			case ASTLiteral::STRING: cout << "\"" << cons->stringValue << "\""; break;
			default: break;
		}
	}

	static void print (ASTRef* ref, int tabs = 0) {
		cout << "(*";
		print(ref->memory, tabs);
		cout << ")";
	}

	static void print (ASTDeref* deref, int tabs = 0) {
		cout << "(&";
		print(deref->memory, tabs);
		cout << ")";
	}

	static void print (ASTCall* call, int tabs = 0) {
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

	static void print (ASTAttr* attr, int tabs = 0) {
		cout << "((";
		print(attr->exp, tabs);
		cout << ") ATTR " << attr->name << ")";
	}

	static void print (ASTUnresolved* unres, int tabs = 0) {
		cout << "(?";
		cout << unres->name;
		cout << "?)";
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
