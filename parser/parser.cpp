#pragma once

#include <stdlib.h>
#include <iostream>
#include <map>

#include "buffer/buffer.cpp"
#include "buffer/file_buffer.cpp"

#include "lexer/lexer.cpp"

#include "ast/ast_type.cpp"
#include "ast/expression/ast_expression.cpp"
#include "ast/statement/ast_statements.cpp"

using namespace std;

#define PARSER_DEBUG false

class Parser {
public:
	Parser (const char* filename) {
		this->lexer = new Lexer(filename);
		this->source = filename;
	}

	Parser (PushbackBuffer* buffer) {
		this->lexer = new Lexer(buffer);
		this->source = "<buffer>";
	}

	ASTType* type () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->nextText();
			auto it = types.find(typeName);
			if (it == types.end()) {
				ASTType* output = new ASTType();
				output->name = typeName;
				return output;
			} else return types[typeName];
		} else return nullptr;
	}

	ASTConst* constant () {
		if (this->lexer->isNextType(Token::Type::STRING)) {
			ASTConst* output = new ASTConst(ASTConst::TYPE::STRING);
			output->stringValue = this->lexer->nextText().c_str();
			return output;
		} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
			ASTConst* output = new ASTConst(ASTConst::TYPE::INT);
			auto text = this->lexer->nextText();
			output->intValue = std::stoi(text, nullptr, 10);
			return output;
		} else return nullptr;
	}

	ASTId* id () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			ASTId* output = new ASTId();
			output->name = this->lexer->nextText();
			return output;
		} else return nullptr;
	}

	ASTFunction* function () {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			ASTFunction* output = new ASTFunction();
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->nextText();
			output->params = this->functionParameters();
			// TODO: get parameter types (optional)
			// TODO: get return type (optional)
			// TODO: get function body (optional)
			return output;
		} else return nullptr;
	}

	ASTExpression* expression () {
	    ASTExpression* lhs = this->term();
	    if (lhs != nullptr) {
	        Token::Type tt = this->lexer->peekType(0);
	        while (tt == Token::Type::ADD || tt == Token::Type::SUB) {
				this->lexer->skip(1);
	            ASTBinop* binop = new ASTBinop(tt);
				binop->lhs = lhs;
	            binop->rhs = this->term();
	            if (binop->rhs == nullptr)
					error("Expected term on the right");
	            lhs = binop;
				tt = this->lexer->peekType(0);
	        }
	        return lhs;
	    } else return nullptr;
	}

	ASTExpression* term () {
		ASTExpression* lhs = this->factor();
		if (lhs != nullptr) {
			Token::Type tt = this->lexer->peekType(0);
			while (tt == Token::Type::MUL || tt == Token::Type::DIV) {
				this->lexer->skip(1);
				ASTBinop* binop = new ASTBinop(tt);
				binop->lhs = lhs;
	            binop->rhs = this->factor();
	            if (binop->rhs == nullptr)
					error("Expected factor on the right");
	            lhs = binop;
				tt = this->lexer->peekType(0);
	        }
	        return lhs;
		} else return nullptr;
	}

	ASTExpression* factor () {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			ASTExpression* exp = this->expression();
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else error("Expected closing parenthesys after expression");
			return exp;
		} else if (this->lexer->isNextType(Token::Type::SUB)) {
			this->lexer->skip(1);
			ASTUnop* _exp = new ASTUnop(Token::Type::SUB);
			_exp->expression = this->factor();
			return (ASTExpression*) _exp;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			return this->expression();
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			return this->id();
		} else if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			return this->function();
		} else return this->constant();
	}

	ASTStatement* statement () {
		ASTStatement* stmt = (ASTStatement*) this->statements();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->var_def();
		if (stmt != nullptr) return stmt;
		return nullptr;
	}

	ASTStatements* statements () {
		if (this->lexer->isNextType(Token::Type::BRAC_OPEN))
			this->lexer->skip(1);
		else return nullptr;

		ASTStatements* output = new ASTStatements();
		ASTStatement* exp = this->statement();
		while (exp != nullptr) {
			output->list.push_back(exp);
			exp = this->statement();
		}

		if (this->lexer->isNextType(Token::Type::BRAC_CLOSE))
			this->lexer->skip(1);
		else error("Expected '}' after statements");

		return output;
	}

	ASTVarDef* var_def () {
		ASTType* type = this->type();
		if (type == nullptr) return nullptr;

		ASTVarDef* output = new ASTVarDef();
		output->type = type;

		if (this->lexer->isNextType(Token::Type::ID))
			output->name = this->lexer->nextText();
		else error("Expected ID after type in variable declaration");

		if (this->lexer->isNextType(Token::Type::EQUAL)) {
			this->lexer->skip(1);
			output->expression = this->expression();
			if (output->expression == nullptr)
				error("Expected expression after equal in variable definition");
		}

		if (this->lexer->isNextType(Token::Type::STM_END))
			this->lexer->skip(1);
		else error("Expected ';' after variable declaration");

		return output;
	}

	ASTStatements* program () {
		ASTStatements* output = new ASTStatements();
		ASTStatement* exp = this->statement();
		while (exp != nullptr) {
			output->list.push_back(exp);
			exp = this->statement();
		}
		return output;
	}

private:
	Token* token = new Token();
	const char* source = nullptr;
	Lexer* lexer = nullptr;

	map<std::string, ASTType*> types;

	vector<ASTVarDef*> functionParameters () {
		vector<ASTVarDef*> output;
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			ASTVarDef* varDef = this->var_def();
			while (varDef != nullptr) {
				output.push_back(varDef);
				varDef = this->var_def();
			}
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else error("Expected closing parenthesys after function parameters");
		}
		return output;
	}

	void error (const char* message) {
		this->lexer->peek(this->token, 0);
		cout << "[Light] ERROR: " << message << endl;
		cout << "        in '" << this->source << "' @ " <<
			this->token->line << ", " << this->token->col << endl;
		exit(EXIT_FAILURE);
	}
};
