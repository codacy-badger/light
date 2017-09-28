#pragma once

#include <stdlib.h>
#include <iostream>
#include <map>

#include "buffer/buffer.cpp"
#include "buffer/file_buffer.cpp"

#include "lexer/lexer.cpp"

#include "ast/type/ast_type.cpp"
#include "ast/expression/ast_expression.cpp"
#include "ast/statement/ast_statement.cpp"
#include "parser_context.cpp"

using namespace std;

class Parser {
public:
	ParserContext* context = new ParserContext();

	Parser (const char* filename) {
		this->initParser(new Lexer(filename), filename);
	}

	Parser (PushbackBuffer* buffer) {
		this->initParser(new Lexer(buffer), "<buffer>");
	}

	ASTType* type () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->text();
			ASTType* output = this->context->getType(typeName);
			if (output == nullptr) {
				cout << "Type " << typeName << " not found!\n";
			} else return output;
		}
		return nullptr;
	}

	ASTConst* constant () {
		if (this->lexer->isNextType(Token::Type::STRING)) {
			ASTConst* output = new ASTConst(ASTConst::TYPE::STRING);
			output->stringValue = this->lexer->text();
			return output;
		} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
			ASTConst* output = new ASTConst(ASTConst::TYPE::INT);
			output->intValue = atoi(this->lexer->text());
			return output;
		} else return nullptr;
	}

	ASTCall* call () {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			ASTCall* output = new ASTCall();
			ASTExpression* exp = this->expression();
			while (exp != nullptr) {
				output->params.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
				exp = this->expression();
			}
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "call arguments");
			return output;
		} else return nullptr;
	}

	ASTVariable* variable () {
		ASTVariable* output = this->id();
		if (output != nullptr) {
			Token::Type tt = this->lexer->nextType;
			while (tt == Token::Type::DOT) {
				this->lexer->skip(1);
				if (tt == Token::Type::DOT) {
					ASTAttr* attr = new ASTAttr(output);
					if (this->lexer->isNextType(Token::Type::ID))
						attr->name = this->lexer->text();
					else expected("name", "attribute access");
					output = attr;
					tt = this->lexer->nextType;
				}
			}
		}
		return output;
	}

	ASTId* id () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			ASTId* output = new ASTId();
			output->name = this->lexer->text();
			return output;
		} else return nullptr;
	}

	ASTExpression* expression (short minPrecedence = 1) {
	    ASTExpression* lhs = this->atom();
	    if (lhs != nullptr) {
	        Token::Type tt = this->lexer->nextType;
			auto precedence = ASTBinop::getPrecedence(tt);
			while (precedence >= minPrecedence) {
				this->lexer->skip(1);

				int nextMinPrec = precedence;
				if (ASTBinop::getLeftAssociativity(tt))
					nextMinPrec += 1;

				ASTBinop* binop = new ASTBinop(tt);
				binop->rhs = this->expression(nextMinPrec);
				binop->lhs = lhs;
				lhs = binop;

				tt = this->lexer->nextType;
				precedence = ASTBinop::getPrecedence(tt);
			}
	        return lhs;
	    } else return nullptr;
	}

	ASTStatement* statement () {
		if (this->lexer->isNextType(Token::Type::STM_END)) {
			this->lexer->skip(1);
			return nullptr;
		}
		ASTStatement* stmt = (ASTStatement*) this->expStm();
		if (stmt != nullptr) return stmt;
		stmt = (ASTDefType*) this->type_def();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->function();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*)this->var_def();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->returnStm();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->statements();
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
		else expected("'}'", "statements");

		return output;
	}

	ASTExpStatement* expStm () {
		ASTExpression* exp = this->expression();
		if (exp == nullptr) return nullptr;
		ASTExpStatement* expStm = new ASTExpStatement();
		expStm->exp = exp;
		if (this->lexer->isNextType(Token::Type::STM_END))
			this->lexer->skip(1);
		else expected("';'", "expression");
		return expStm;
	}

	ASTDefType* type_def () {
		if (this->lexer->isNextType(Token::Type::TYPE)) {
			this->lexer->skip(1);
			ASTDefType* output = new ASTDefType();
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->text();
			else expected("Identifier", "'type' keyword");
			output->stms = this->statements();
			return output;
		} else return nullptr;
	}

	ASTVarDef* var_def () {
		if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);

			ASTVarDef* output = new ASTVarDef();
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->text();
			else expected("id", "'let'");

			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				output->type = this->type();
				if (output->type == nullptr)
					expected("type", "':'");
			} else expected("':'", "variable name");

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == nullptr)
					expected("expression", "'='");
			}

			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else error("Expected ';' after variable declaration");
			return output;
		} else return nullptr;
	}

	ASTReturn* returnStm () {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			ASTReturn* output = new ASTReturn();
			output->expression = this->expression();
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "return expression");
			return output;
		} else return nullptr;
	}

	ASTFunction* function () {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			ASTFunction* output = new ASTFunction();
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->text();
			else expected("Identifier", "'fn' keyword");
			output->fnType = this->functionType();
			output->stms = this->statement();
			return output;
		} else return nullptr;
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
	const char* source = nullptr;
	Lexer* lexer = nullptr;

	void initParser (Lexer* lexer, const char* source) {
		this->source = source;
		this->lexer = lexer;
	}

	ASTExpression* atom () {
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
			_exp->expression = this->atom();
			return (ASTExpression*) _exp;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			return this->expression();
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			ASTVariable* var = this->variable();
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				ASTCall* fnCall = this->call();
				fnCall->var = var;
				return fnCall;
			} else return var;
		} else return this->constant();
	}

	ASTVarDef* _var_def () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			ASTVarDef* output = new ASTVarDef();
			output->name = this->lexer->text();

			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				output->type = this->type();
				// TODO: remove this once we have type inference
				if (output->type == nullptr) expected("type", "':'");
			} else expected("':'", "variable name");

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == nullptr)
					expected("expression", "'='");
			}
			return output;
		} else return nullptr;
	}

	ASTFnType* functionType () {
		ASTFnType* output = new ASTFnType();
		output->params = this->functionParameters();
		if (this->lexer->isNextType(Token::Type::ARROW)) {
			this->lexer->skip(1);
			output->retType = this->type();
		}
		return output;
	}

	vector<ASTVarDef*> functionParameters () {
		vector<ASTVarDef*> output;
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			ASTVarDef* fnParam = this->_var_def();
			while (fnParam != nullptr) {
				output.push_back(fnParam);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
				fnParam = this->_var_def();
			}
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "function parameters");
		}
		return output;
	}

	void error (const char* message) {
		cout << "[Light] ERROR: " << message << endl;
		cout << "        in '" << this->source << "' @ " <<
			this->lexer->buffer->line << ", " <<
			this->lexer->buffer->col << endl;
		exit(EXIT_FAILURE);
	}

	void expected (const char* expect, const char* after) {
		cout << "[Light] ERROR: Expected " << expect
			<< " after " << after << endl;
		cout << "        in '" << this->source << "' @ " <<
			this->lexer->buffer->line << ", " <<
			this->lexer->buffer->col << endl;
		exit(EXIT_FAILURE);
	}
};
