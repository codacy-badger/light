#pragma once

#include "ast.h"

class ASTPrinter {
public:
	static void print (ASTType* type, int tabs = 0) {
		if (type->name == "") std::cout  << "[?]";
		else std::cout << type->name;
	}

	static void print (ASTStatement* stm, int tabs = 0) {
		if (auto obj = dynamic_cast<ASTVarDef*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTStatements*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTFunction*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTReturn*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTDefType*>(stm)) print(obj, tabs);
		else if (auto obj = dynamic_cast<ASTExpStatement*>(stm)) print(obj, tabs);
		else _panic("Unrecognized statement?!");
	}

	static void print (ASTVarDef* stm, int tabs = 0, bool simple = false) {
		if (!simple) {
			_tabs(tabs);
			cout << "LET ";
		}

		cout << stm->name << " : ";
		print(stm->type, tabs);
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
		if (stm->fnType->params.size() > 0) {
			print(stm->fnType->params[0], tabs, true);
			for (int i = 1; i < stm->fnType->params.size(); i++) {
				cout << ", ";
				print(stm->fnType->params[i], tabs, true);
			}
		}
		cout << " )";
		if (stm->fnType->retType != nullptr) {
			cout << " -> ";
			print(stm->fnType->retType, tabs);
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

	static void print (ASTDefType* type, int tabs = 0) {
		cout << "type " << type->name << "\n";
		if (type->stms != nullptr)
			print(type->stms, tabs + 1);
	}

	static void print (ASTExpStatement* expStm, int tabs = 0) {
		_tabs(tabs);
		cout << "EXP ";
		print(expStm->exp, tabs);
		cout << "\n";
	}

	static void print (ASTExpression* exp, int tabs = 0) {
		if (typeid(*exp) == typeid(ASTBinop))
			print(static_cast<ASTBinop*>(exp), tabs);
		else if (typeid(*exp) == typeid(ASTUnop))
			print(static_cast<ASTUnop*>(exp), tabs);
		else if (typeid(*exp) == typeid(ASTConst))
			print(static_cast<ASTConst*>(exp), tabs);
		else if (typeid(*exp) == typeid(ASTCall))
			print(static_cast<ASTCall*>(exp), tabs);
		else if (typeid(*exp) == typeid(ASTAttr))
			print(static_cast<ASTAttr*>(exp), tabs);
		else if (typeid(*exp) == typeid(ASTId))
			print(static_cast<ASTId*>(exp), tabs);
		else _panic("Unrecognized expression?!");
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

	static void print (ASTId* id, int tabs = 0) {
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
