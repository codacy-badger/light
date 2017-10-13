#pragma once

#include <map>
#include <vector>
#include <string>
#include "lexer/token.cpp"

struct Ast_Type_Definition;
struct ASTBlock;
struct ASTFunction;
struct ASTExpression;
struct ASTVariable;

using namespace std;

enum Ast_Type {
	AST_STATEMENT,
	AST_BLOCK,
	AST_DECLARATION,
	AST_RETURN,

	AST_EXPRESSION,
	AST_FUNCTION,
	AST_TYPE_DEFINITION,
	AST_LITERAL,
	AST_BINARY,
	AST_UNARY,
	AST_CALL,
};

struct AST {
	Ast_Type type;

	const char* filename;
	long line, col;

	virtual ~AST () {}
};

struct ASTStatement : AST {
	ASTStatement() { this->type = AST_STATEMENT; }
	virtual ~ASTStatement() {}
};

struct ASTBlock : ASTStatement {
	string name;
	vector<ASTStatement*> list;

	ASTBlock* parent = nullptr;
	map<string, ASTExpression*> symbols;

	ASTBlock (string name, ASTBlock* parent = nullptr) {
		this->type = AST_BLOCK;
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

	ASTReturn() { this->type = AST_RETURN; }
};

struct ASTExpression : ASTStatement {
	ASTExpression() { this->type = AST_EXPRESSION; }
	virtual ~ASTExpression() {}
};

struct Ast_Type_Definition : ASTExpression {
	vector<ASTVariable*> attrs;
	vector<ASTFunction*> methods;
};

struct ASTPointerType : Ast_Type_Definition {
	Ast_Type_Definition* base = nullptr;
};

struct ASTFnType : Ast_Type_Definition {
	vector<ASTVariable*> params;
	Ast_Type_Definition* retType = nullptr;
};

struct ASTStructType : Ast_Type_Definition {
	string name;

	ASTStructType (string name = "") { this->name = name; }
};

struct ASTPrimitiveType : Ast_Type_Definition {
	string name;

	ASTPrimitiveType (string name) { this->name = name; }

	static Ast_Type_Definition* _void;
	static Ast_Type_Definition* _i1;
	static Ast_Type_Definition* _i8;
	static Ast_Type_Definition* _i16;
	static Ast_Type_Definition* _i32;
	static Ast_Type_Definition* _i64;
	static Ast_Type_Definition* _i128;
};

Ast_Type_Definition* ASTPrimitiveType::_void = new ASTPrimitiveType("void");
Ast_Type_Definition* ASTPrimitiveType::_i1 =   new ASTPrimitiveType("i1");
Ast_Type_Definition* ASTPrimitiveType::_i8 =   new ASTPrimitiveType("i8");
Ast_Type_Definition* ASTPrimitiveType::_i16 =  new ASTPrimitiveType("i16");
Ast_Type_Definition* ASTPrimitiveType::_i32 =  new ASTPrimitiveType("i32");
Ast_Type_Definition* ASTPrimitiveType::_i64 =  new ASTPrimitiveType("i64");
Ast_Type_Definition* ASTPrimitiveType::_i128 = new ASTPrimitiveType("i128");

struct ASTFunction : ASTExpression {
	string name;
	ASTFnType* type = nullptr;
	ASTStatement* stm = nullptr;
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
};

map<ASTUnop::OP, const char*> ASTUnop::opChar = {
	{ASTUnop::OP::NEG, "-"}
};

struct ASTValue : ASTExpression {
};

struct ASTCall : ASTValue {
	ASTExpression* fn;
	vector<ASTExpression*> params;
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
};

struct ASTMemory : ASTValue {
	~ASTMemory () {}
};

struct ASTVariable : ASTMemory {
	string name = "";
	Ast_Type_Definition* type = nullptr;
	ASTExpression* expression = nullptr;

	bool isConstant() { return false; }
};

struct ASTRef : ASTMemory {
	ASTMemory* memory = nullptr;
};

struct ASTDeref : ASTMemory {
	ASTMemory* memory = nullptr;
};

struct ASTAttr : ASTMemory {
	ASTExpression* exp = nullptr;
	string name;

	ASTAttr (ASTExpression* exp = nullptr) {
		this->exp = exp;
	}
};

struct ASTUnresolved {
	string name;

	ASTUnresolved (string name = "") {
		this->name = name;
	}
};

struct ASTUnresolvedExp : ASTUnresolved, ASTExpression {
	ASTUnresolvedExp (string name = "") : ASTUnresolved(name) { /* empty */ }
};

struct ASTUnresolvedTy : ASTUnresolved, Ast_Type_Definition {
	ASTUnresolvedTy (string name = "") : ASTUnresolved(name) { /* empty */ }
};

struct ASTUnresolvedFn : ASTUnresolved, ASTFunction {
	ASTUnresolvedFn (string name = "") : ASTUnresolved(name) { /* empty */ }
};
