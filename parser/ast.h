#pragma once

#include <map>
#include <vector>
#include <string>
#include "lexer/token.cpp"

struct ASTType;
struct ASTBlock;
struct ASTFunction;
struct ASTExpression;
struct ASTVariable;

using namespace std;

enum Ast_Type {
	Ast_Statement,
	Ast_Block,
	Ast_Declaration,
	Ast_Return,

	Ast_Type_Instance,

	Ast_Expression,
	Ast_Function,
	Ast_Type_Declaration,
	Ast_Literal,
	Ast_Binary,
	Ast_Unary,
	Ast_Call,
};

struct AST {
	Ast_Type type;

	const char* filename;
	long line, col;

	virtual ~AST () {}
};

struct ASTStatement : AST {
	ASTStatement() {this->type = Ast_Statement;}
	virtual ~ASTStatement() {}
};

struct ASTBlock : ASTStatement {
	string name;
	vector<ASTStatement*> list;

	ASTBlock* parent = nullptr;
	map<string, ASTExpression*> symbols;

	ASTBlock (string name, ASTBlock* parent = nullptr) {
		this->type = Ast_Block;
		this->parent = parent;
		this->name = name;
	}

	void add (string name, ASTExpression* val) {
		auto it = this->symbols.find(name);
		if (it == this->symbols.end()) {
			this->symbols[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
			exit(1);
		}
	}

	ASTExpression* get (string name) {
		auto it = this->symbols.find(name);
		if (it != this->symbols.end()) {
			return this->symbols[name];
		} else {
			if (this->parent)
				return this->parent->get(name);
			else return nullptr;
		}
	}

	template <typename T>
	T* get (string name) {
		if (auto casted = dynamic_cast<T*>(this->get(name)))
			return casted;
		else return nullptr;
	}
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;

	ASTReturn() {this->type = Ast_Return;}
};

struct ASTExpression : ASTStatement {
	ASTExpression() {this->type = Ast_Expression;}
	virtual ~ASTExpression() {}
	virtual ASTType* getType() = 0;
};

struct ASTType : ASTExpression {
	vector<ASTVariable*> attrs;
	vector<ASTFunction*> methods;

	virtual ASTType* getType() {
		//TODO: return special Type for consistency
		return nullptr;
	}
};

struct ASTPointerType : ASTType {
	ASTType* base = nullptr;
};

struct ASTFnType : ASTType {
	vector<ASTVariable*> params;
	ASTType* retType = nullptr;
};

struct ASTStructType : ASTType {
	string name;

	ASTStructType (string name = "") { this->name = name; }
};

struct ASTPrimitiveType : ASTType {
	string name;

	ASTPrimitiveType (string name) { this->name = name; }

	static ASTType* _void;
	static ASTType* _i1;
	static ASTType* _i8;
	static ASTType* _i16;
	static ASTType* _i32;
	static ASTType* _i64;
	static ASTType* _i128;
};

ASTType* ASTPrimitiveType::_void = new ASTPrimitiveType("void");
ASTType* ASTPrimitiveType::_i1 =   new ASTPrimitiveType("i1");
ASTType* ASTPrimitiveType::_i8 =   new ASTPrimitiveType("i8");
ASTType* ASTPrimitiveType::_i16 =  new ASTPrimitiveType("i16");
ASTType* ASTPrimitiveType::_i32 =  new ASTPrimitiveType("i32");
ASTType* ASTPrimitiveType::_i64 =  new ASTPrimitiveType("i64");
ASTType* ASTPrimitiveType::_i128 = new ASTPrimitiveType("i128");

struct ASTFunction : ASTExpression {
	string name;
	ASTFnType* type = nullptr;
	ASTStatement* stm = nullptr;

	ASTType* getType() { return this->type; }
};

struct ASTBinop : ASTExpression {
	enum OP { ASSIGN, ATTR, ADD, SUB, MUL, DIV, COUNT };
	static map<Token::Type, bool> isLeftAssociate;
	static map<ASTBinop::OP, const char*> opChar;
	static map<Token::Type, short> precedence;

	OP op = OP::COUNT;
	ASTExpression* lhs = nullptr;
	ASTExpression* rhs = nullptr;

	ASTBinop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OP typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::EQUAL: return OP::ASSIGN;
			case Token::Type::DOT: return OP::ATTR;
			case Token::Type::ADD: return OP::ADD;
			case Token::Type::SUB: return OP::SUB;
			case Token::Type::MUL: return OP::MUL;
			case Token::Type::DIV: return OP::DIV;
			default:
				cout << "[ERROR] Binary operator unknown: " << tType << "\n";
				exit(1);
		};
		return OP::COUNT;
	}

	ASTType* getType() {
		ASTType* lhsTy = this->lhs->getType();
		ASTType* rhsTy = this->rhs->getType();
		if (lhsTy == rhsTy) return lhsTy;
		else return nullptr;
	}

	static short getPrecedence (Token::Type opToken) {
		auto it = ASTBinop::precedence.find(opToken);
		if (it != ASTBinop::precedence.end())
			return ASTBinop::precedence[opToken];
		else return -1;
	}

	static bool getLeftAssociativity (Token::Type opToken) {
		auto it = ASTBinop::isLeftAssociate.find(opToken);
		if (it != ASTBinop::isLeftAssociate.end())
			return ASTBinop::isLeftAssociate[opToken];
		else return false;
	}
};

map<ASTBinop::OP, const char*> ASTBinop::opChar = {
	{ASTBinop::OP::ASSIGN, "="}, {ASTBinop::OP::ATTR, "."},
	{ASTBinop::OP::ADD, "+"}, {ASTBinop::OP::SUB, "-"},
	{ASTBinop::OP::MUL, "*"}, {ASTBinop::OP::DIV, "/"}
};
map<Token::Type, short> ASTBinop::precedence = {
	{Token::Type::EQUAL, 1}, {Token::Type::DOT, 1},
	{Token::Type::ADD, 2}, {Token::Type::SUB, 2},
	{Token::Type::MUL, 3}, {Token::Type::DIV, 3}
};
map<Token::Type, bool> ASTBinop::isLeftAssociate = {
	{Token::Type::EQUAL, false}, {Token::Type::DOT, false},
	{Token::Type::ADD, false}, {Token::Type::SUB, false},
	{Token::Type::MUL, false}, {Token::Type::DIV, false}
};

struct ASTUnop : ASTExpression {
	enum OP { NEG, COUNT };
	static map<ASTUnop::OP, const char*> opChar;

	OP op = OP::COUNT;
	ASTExpression* exp = nullptr;

	ASTUnop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OP typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::SUB: return OP::NEG;
			default:
				cout << "PANIC -> " << tType << "\n";
				exit(87);
		};
		return OP::COUNT;
	}

	ASTType* getType() {
		return this->exp->getType();
	}
};

map<ASTUnop::OP, const char*> ASTUnop::opChar = {
	{ASTUnop::OP::NEG, "-"}
};

struct ASTValue : ASTExpression {
};

struct ASTCall : ASTValue {
	ASTExpression* fn;
	vector<ASTExpression*> params;

	ASTType* getType() {
		if (auto _fn = dynamic_cast<ASTFunction*>(fn)) {
			return _fn->type->retType;
		} else return nullptr;
	}
};

struct ASTLiteral : ASTValue {
	enum TYPE { BYTE, SHORT, INT, LONG, FLOAT, DOUBLE, STRING, COUNT };
	TYPE type = TYPE::COUNT;
	union {
		int8_t byteValue;
		int16_t shortValue;
		int32_t intValue;
		int64_t longValue;
		float floatValue;
		double doubleValue;
		char* stringValue;
	};

	ASTLiteral (TYPE type) {
		this->type = type;
	}

	ASTType* getType() {
		switch (type) {
			case BYTE:   	return nullptr;
			case SHORT:  	return nullptr;
			case INT:    	return ASTPrimitiveType::_i32;
			case LONG:   	return nullptr;
			case FLOAT:  	return nullptr;
			case DOUBLE: 	return nullptr;
			case STRING: 	return nullptr;
			default:		return nullptr;
		}
	}
};

struct ASTMemory : ASTValue {
	~ASTMemory () {}
};

struct ASTVariable : ASTMemory {
	string name = "";
	ASTType* type = nullptr;
	ASTExpression* expression = nullptr;

	bool isConstant() { return false; }
	ASTType* getType() {
		return this->type;
	}
};

struct ASTRef : ASTMemory {
	ASTMemory* memory = nullptr;

	ASTType* getType() {
		// TODO: return some sort of custom pointer type
		return nullptr;
	}
};

struct ASTDeref : ASTMemory {
	ASTMemory* memory = nullptr;

	ASTType* getType() {
		// TODO: return some sort of custom pointer type
		return nullptr;
	}
};

struct ASTAttr : ASTMemory {
	ASTExpression* exp = nullptr;
	string name;

	ASTAttr (ASTExpression* exp = nullptr) {
		this->exp = exp;
	}

	ASTType* getType() {
		// TODO: store variables in context to query type
		return nullptr;
	}
};

struct ASTUnresolved {
	string name;

	ASTUnresolved (string name = "") {
		this->name = name;
	}
};

struct ASTUnresolvedExp : ASTUnresolved, ASTExpression {

	ASTUnresolvedExp (string name = "") : ASTUnresolved(name)
	{ /* empty */ }

	ASTType* getType() { return nullptr; }
};

struct ASTUnresolvedTy : ASTUnresolved, ASTType {

	ASTUnresolvedTy (string name = "") : ASTUnresolved(name)
	{ /* empty */ }
};

struct ASTUnresolvedFn : ASTUnresolved, ASTFunction {

	ASTUnresolvedFn (string name = "") : ASTUnresolved(name)
	{ /* empty */ }
};
