#pragma once

#include <map>
#include <vector>
#include <string>
#include "lexer/token.cpp"

struct Ast_Type_Definition;
struct Ast_Block;
struct Ast_Function;
struct Ast_Expression;
struct Ast_Variable;

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
};

struct Ast_Statement : AST {
	Ast_Statement() { this->type = AST_STATEMENT; }
	virtual ~Ast_Statement() {}
};

struct Ast_Block : Ast_Statement {
	string name;
	vector<Ast_Statement*> list;

	Ast_Block* parent = nullptr;
	map<string, Ast_Expression*> symbols;

	Ast_Block (string name, Ast_Block* parent = nullptr) {
		this->type = AST_BLOCK;
		this->parent = parent;
		this->name = name;
	}

	void add (string name, Ast_Expression* val) {
		auto it = this->symbols.find(name);
		if (it == this->symbols.end()) {
			this->symbols[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
			exit(1);
		}
	}

	Ast_Expression* get (string name) {
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

struct Ast_Return : Ast_Statement {
	Ast_Expression* exp = nullptr;

	Ast_Return() { this->type = AST_RETURN; }
};

struct Ast_Expression : Ast_Statement {
	Ast_Expression() { this->type = AST_EXPRESSION; }
	virtual ~Ast_Expression() {}
};

struct Ast_Type_Definition : Ast_Expression {
	vector<Ast_Variable*> attrs;
	vector<Ast_Function*> methods;
};

struct Ast_Pointer_Type : Ast_Type_Definition {
	Ast_Type_Definition* base = nullptr;
};

struct Ast_Function_Type : Ast_Type_Definition {
	vector<Ast_Variable*> params;
	Ast_Type_Definition* retType = nullptr;
};

struct Ast_Struct_Type : Ast_Type_Definition {
	string name;

	Ast_Struct_Type (string name = "") { this->name = name; }
};

struct Ast_Primitive_Type : Ast_Type_Definition {
	string name;

	Ast_Primitive_Type (string name) { this->name = name; }

	static Ast_Type_Definition* _void;
	static Ast_Type_Definition* _i1;
	static Ast_Type_Definition* _i8;
	static Ast_Type_Definition* _i16;
	static Ast_Type_Definition* _i32;
	static Ast_Type_Definition* _i64;
	static Ast_Type_Definition* _i128;
};

Ast_Type_Definition* Ast_Primitive_Type::_void = new Ast_Primitive_Type("void");
Ast_Type_Definition* Ast_Primitive_Type::_i1 =   new Ast_Primitive_Type("i1");
Ast_Type_Definition* Ast_Primitive_Type::_i8 =   new Ast_Primitive_Type("i8");
Ast_Type_Definition* Ast_Primitive_Type::_i16 =  new Ast_Primitive_Type("i16");
Ast_Type_Definition* Ast_Primitive_Type::_i32 =  new Ast_Primitive_Type("i32");
Ast_Type_Definition* Ast_Primitive_Type::_i64 =  new Ast_Primitive_Type("i64");
Ast_Type_Definition* Ast_Primitive_Type::_i128 = new Ast_Primitive_Type("i128");

struct Ast_Function : Ast_Expression {
	string name;
	Ast_Function_Type* type = nullptr;
	Ast_Statement* stm = nullptr;
};

enum Ast_Binary_Type {
	AST_BINARY_UNINITIALIZED,
	AST_BINARY_ASSIGN,
	AST_BINARY_ATTRIBUTE,
	AST_BINARY_ADD,
	AST_BINARY_SUB,
	AST_BINARY_MUL,
	AST_BINARY_DIV,
};

struct AST_Binary : Ast_Expression {
	static map<Token::Type, bool> isLeftAssociate;
	static map<Ast_Binary_Type, const char*> opChar;
	static map<Token::Type, short> precedence;

	Ast_Binary_Type op = AST_BINARY_UNINITIALIZED;
	Ast_Expression* lhs = nullptr;
	Ast_Expression* rhs = nullptr;

	AST_Binary (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	Ast_Binary_Type typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::EQUAL: 	return AST_BINARY_ASSIGN;
			case Token::Type::DOT: 		return AST_BINARY_ATTRIBUTE;
			case Token::Type::ADD: 		return AST_BINARY_ADD;
			case Token::Type::SUB: 		return AST_BINARY_SUB;
			case Token::Type::MUL: 		return AST_BINARY_MUL;
			case Token::Type::DIV: 		return AST_BINARY_DIV;
			default: {
				cout << "[ERROR] Binary operator unknown: " << tType << "\n";
				return AST_BINARY_UNINITIALIZED;
			}
		};
	}

	static short getPrecedence (Token::Type opToken) {
		auto it = AST_Binary::precedence.find(opToken);
		if (it != AST_Binary::precedence.end())
			return AST_Binary::precedence[opToken];
		else return -1;
	}

	static bool getLeftAssociativity (Token::Type opToken) {
		auto it = AST_Binary::isLeftAssociate.find(opToken);
		if (it != AST_Binary::isLeftAssociate.end())
			return AST_Binary::isLeftAssociate[opToken];
		else return false;
	}
};

map<Ast_Binary_Type, const char*> AST_Binary::opChar = {
	{AST_BINARY_ASSIGN, "="}, 	{AST_BINARY_ATTRIBUTE, 	"."},
	{AST_BINARY_ADD, 	"+"}, 	{AST_BINARY_SUB, 		"-"},
	{AST_BINARY_MUL, 	"*"}, 	{AST_BINARY_DIV, 		"/"}
};
map<Token::Type, short> AST_Binary::precedence = {
	{Token::Type::EQUAL, 	1}, {Token::Type::DOT, 1},
	{Token::Type::ADD, 		2}, {Token::Type::SUB, 2},
	{Token::Type::MUL, 		3}, {Token::Type::DIV, 3}
};
map<Token::Type, bool> AST_Binary::isLeftAssociate = {
	{Token::Type::EQUAL, 	false}, {Token::Type::DOT, false},
	{Token::Type::ADD, 		false}, {Token::Type::SUB, false},
	{Token::Type::MUL, 		false}, {Token::Type::DIV, false}
};

struct AST_Unary : Ast_Expression {
	enum OP { NEG, COUNT };
	static map<AST_Unary::OP, const char*> opChar;

	OP op = OP::COUNT;
	Ast_Expression* exp = nullptr;

	AST_Unary (Token::Type tType) {
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

map<AST_Unary::OP, const char*> AST_Unary::opChar = {
	{AST_Unary::OP::NEG, "-"}
};

struct Ast_Value : Ast_Expression {
};

struct Ast_Function_Call : Ast_Value {
	Ast_Expression* fn;
	vector<Ast_Expression*> params;
};

struct Ast_Literal : Ast_Value {
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

	Ast_Literal (TYPE type) {
		this->type = type;
	}
};

struct AST_Memory : Ast_Value {
	~AST_Memory () {}
};

struct Ast_Variable : AST_Memory {
	string name = "";
	Ast_Type_Definition* type = nullptr;
	Ast_Expression* expression = nullptr;

	bool isConstant() { return false; }
};

struct AST_Ref : AST_Memory {
	AST_Memory* memory = nullptr;
};

struct Ast_Deref : AST_Memory {
	AST_Memory* memory = nullptr;
};

struct Ast_Attribute : AST_Memory {
	Ast_Expression* exp = nullptr;
	string name;

	Ast_Attribute (Ast_Expression* exp = nullptr) {
		this->exp = exp;
	}
};

struct Ast_Unresolved {
	string name;

	Ast_Unresolved (string name = "") {
		this->name = name;
	}
};

struct Ast_Unresolved_Expression : Ast_Unresolved, Ast_Expression {
	Ast_Unresolved_Expression (string name = "") : Ast_Unresolved(name) { /* empty */ }
};

struct Ast_Unresolved_Type : Ast_Unresolved, Ast_Type_Definition {
	Ast_Unresolved_Type (string name = "") : Ast_Unresolved(name) { /* empty */ }
};

struct Ast_Unresolved_Function : Ast_Unresolved, Ast_Function {
	Ast_Unresolved_Function (string name = "") : Ast_Unresolved(name) { /* empty */ }
};
