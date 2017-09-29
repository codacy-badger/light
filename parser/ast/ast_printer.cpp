#pragma once

#include "ast.h"

class ASTPrinter {
public:

	static void print (ASTStatement* stm, int tabs = 0) {
		if 		(auto obj = dynamic_cast<ASTVarDef*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTStatements*>(stm)) print(obj, tabs);
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

	static void print (ASTVarDef* stm, int tabs = 0, bool simple = false) {
		if (!simple) {
			_tabs(tabs);
			cout << "LET ";
		}

		cout << stm->name << " : ";
		print(stm->type, tabs, true);
		if (stm->expression != nullptr) {
			cout << " = ";
			print(stm->expression, tabs);
		}

		if (!simple) cout << "\n";
	}

	static void print (ASTStatements* stm, int tabs = 0) {
		for(auto const& stm: stm->list) {
			print(stm, tabs);
		}
	}

	static void print (ASTFunction* stm, int tabs = 0) {
		cout << "fn " << stm->name << " ";

		cout << "( ";
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
			print(stm->type->retType, tabs);
		}

		cout << "\n";
		if (stm->stms != nullptr)
			print(stm->stms, tabs + 1);
	}

	static void print (ASTReturn* ret, int tabs = 0) {
		_tabs(tabs);
		cout << "RETURN ";
		if (ret->exp == nullptr) cout << "void";
		else print(ret->exp, tabs);
		cout << endl;
	}

	static void print (ASTType* type, int tabs = 0, bool nameOnly = false) {
		if (!nameOnly) cout << "type ";
		cout << type->name;
		if (!nameOnly && type->stms != nullptr) {
			cout << "\n";
			print(type->stms, tabs + 1);
		}
	}

	static void print (ASTExpression* exp, int tabs = 0) {
		if 		(auto obj = dynamic_cast<ASTBinop*>(exp))   print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTUnop*>(exp))    print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTConst*>(exp))   print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTCall*>(exp))    print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTAttr*>(exp))    print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTPointer*>(exp)) print(obj, tabs);
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

	static void print (ASTConst* cons, int tabs = 0) {
		switch (cons->type) {
			case ASTConst::BYTE:   cout << cons->byteValue;   break;
			case ASTConst::SHORT:  cout << cons->shortValue;  break;
			case ASTConst::INT:    cout << cons->intValue;    break;
			case ASTConst::LONG:   cout << cons->longValue;   break;
			case ASTConst::FLOAT:  cout << cons->floatValue;  break;
			case ASTConst::DOUBLE: cout << cons->doubleValue; break;
			case ASTConst::STRING: cout << "\"" << cons->stringValue << "\""; break;
			default: break;
		}
	}

	static void print (ASTCall* call, int tabs = 0) {
		cout << "(";
		print(call->var, tabs);
		cout << " CALL ( ";
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

	static void print (ASTPointer* id, int tabs = 0) {
		cout << "[" << id->name << "]";
	}
private:
	static void _tabs (int count) {
		for (int i = 0; i < count; i++)
			cout << "    ";
	}

	static void _panic (const char* message) {

	}
};
