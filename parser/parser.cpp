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
			} else return NULL;
		}

		ASTId* id () {
			if (this->lexer->isNextType(Token::Type::ID)) {
				ASTId* output = new ASTId();
				output->name = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTString* string () {
			if (this->lexer->isNextType(Token::Type::STRING)) {
				ASTString* output = new ASTString();
				output->value = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTNumber* number () {
			if (this->lexer->isNextType(Token::Type::NUMBER)) {
				ASTNumber* output = new ASTNumber();
				output->value = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTExpression* expression () {
		    ASTExpression* lhs = this->term();
		    if (lhs != NULL) {
		        Token::Type tt = this->lexer->peekType(0);
		        while (tt == Token::Type::ADD || tt == Token::Type::SUB) {
					this->lexer->skip(1);
		            ASTExpressionBinop* binop = NULL;
		            if (tt == Token::Type::ADD) {
						binop = new ASTExpressionBinopAdd();
		            } else binop = new ASTExpressionBinopSub();
					binop->lhs = lhs;
		            binop->rhs = this->term();
		            if (binop->rhs == NULL)
						error("Expected term on the right");
		            lhs = binop;
					tt = this->lexer->peekType(0);
		        }
		        return lhs;
		    } else return NULL;
		}

		ASTExpression* term () {
			ASTExpression* lhs = this->factor();
			if (lhs != NULL) {
				Token::Type tt = this->lexer->peekType(0);
				while (tt == Token::Type::MUL || tt == Token::Type::DIV) {
					this->lexer->skip(1);
					ASTExpressionBinop* binop = NULL;
		            if (tt == Token::Type::MUL) {
						binop = new ASTExpressionBinopMul();
		            } else binop = new ASTExpressionBinopDiv();
					binop->lhs = lhs;
		            binop->rhs = this->factor();
		            if (binop->rhs == NULL)
						error("Expected factor on the right");
		            lhs = binop;
					tt = this->lexer->peekType(0);
		        }
		        return lhs;
			} else return NULL;
		}

		ASTExpression* factor () {
			ASTExpression* exp = NULL;

			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				this->lexer->skip(1);
				exp = this->expression();
				if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
					this->lexer->skip(1);
				else error("Expected closing parenthesys after expression");
			} else if (this->lexer->isNextType(Token::Type::SUB)) {
				this->lexer->skip(1);
				ASTExpressionUnopNeg* _exp = new ASTExpressionUnopNeg();
				_exp->expression = this->factor();
				exp = (ASTExpression*) _exp;
			} else if (this->lexer->isNextType(Token::Type::ADD)) {
				this->lexer->skip(1);
				exp = this->expression();
			} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
				exp = this->number();
			} else if (this->lexer->isNextType(Token::Type::ID)) {
				exp = this->id();
			} else if (this->lexer->isNextType(Token::Type::STRING)) {
				exp = this->string();
			}

			return exp;
		}

		ASTStatement* statement () {
			ASTStatement* stmt = (ASTStatement*) this->statements();
			if (stmt != NULL) return stmt;
			stmt = (ASTStatement*) this->statement_def_variable();
			if (stmt != NULL) return stmt;
			return NULL;
		}

		ASTStatements* statements () {
			if (this->lexer->isNextType(Token::Type::BRAC_OPEN))
				this->lexer->skip(1);
			else return NULL;

			ASTStatements* output = new ASTStatements();
			ASTStatement* exp = this->statement();
			while (exp != NULL) {
				output->list.push_back(exp);
				exp = this->statement();
			}

			if (this->lexer->isNextType(Token::Type::BRAC_CLOSE))
				this->lexer->skip(1);
			else error("Expected '}' after statements");

			return output;
		}

		ASTVarDef* statement_def_variable () {
			ASTType* type = this->type();
			if (type == NULL) return NULL;

			ASTVarDef* output = new ASTVarDef();
			output->type = type;

			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->nextText();
			else error("Expected ID after type in variable declaration");

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == NULL)
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
			while (exp != NULL) {
				output->list.push_back(exp);
				exp = this->statement();
			}
			return output;
		}

	private:
		Token* token = new Token();
		const char* source = NULL;
		Lexer* lexer = NULL;

		map<std::string, ASTType*> types;

		void error (const char* message) {
			this->lexer->peek(this->token, 0);
			cout << "[Light] ERROR: " << message << endl;
			cout << "        in '" << this->source << "' @ " <<
				this->token->line << ", " << this->token->col << endl;
			exit(EXIT_FAILURE);
		}
};
