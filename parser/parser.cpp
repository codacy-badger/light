#pragma once

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "buffer/buffer.cpp"
#include "buffer/file_buffer.cpp"

#include "lexer/lexer.cpp"

#include "ast/ast.hpp"

using namespace std;

class Parser {
public:
	ASTContext* context = new ASTContext();

	Parser (const char* filename) {
		this->initParser(new Lexer(filename), filename);
	}

	Parser (PushbackBuffer* buffer) {
		this->initParser(new Lexer(buffer), "<buffer>");
	}

	ASTType* type () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->nextText;
			auto exp = this->context->get(typeName);
			if (exp != nullptr) this->lexer->skip(1);
			return dynamic_cast<ASTType*>(exp);
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

	ASTExpression* variable () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			string name(this->lexer->nextText);
			auto output = dynamic_cast<ASTExpression*>(this->context->get(name));
			if (output != nullptr) this->lexer->skip(1);

			Token::Type tt = this->lexer->nextType;
			while (tt == Token::Type::DOT) {
				this->lexer->skip(1);
				if (tt == Token::Type::DOT) {
					auto attr = new ASTAttr(output);
					if (this->lexer->isNextType(Token::Type::ID))
						attr->name = this->lexer->text();
					else expected("name", "attribute access");
					output = attr;
					tt = this->lexer->nextType;
				}
			}

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

				ASTBinop* output = new ASTBinop(tt);
				output->rhs = this->expression(nextMinPrec);
				output->lhs = lhs;
				lhs = output;

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

		ASTStatement* stmt = (ASTType*) this->type_def();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->function();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*)this->var_def();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->returnStm();
		if (stmt != nullptr) return stmt;
		stmt = (ASTStatement*) this->statements();
		if (stmt != nullptr) return stmt;

		stmt = (ASTStatement*) this->expression();
		if (stmt != nullptr) {
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "expression");
			return stmt;
		} else return nullptr;
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

	ASTStructType* type_def () {
		if (this->lexer->isNextType(Token::Type::TYPE)) {
			this->lexer->skip(1);
			auto output = new ASTStructType(this->lexer->text());
			auto stms = this->statements();
			if (stms != nullptr) {
				for (auto const &it : stms->list) {
					if (auto var = dynamic_cast<ASTVariable*>(it))
						output->attrs.push_back(var);
					else if (auto fn = dynamic_cast<ASTFunction*>(it))
						output->methods.push_back(fn);
					else error("Expected attribute or method inside type");
				}
			} else this->lexer->skip(1);
			this->context->add(output->name, output);
			return output;
		} else return nullptr;
	}

	ASTVariable* var_def () {
		if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);

			auto output = this->_var_def();
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "variable declaration");

			this->context->add(output->name, output);
			return output;
		} else return nullptr;
	}

	ASTReturn* returnStm () {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			ASTReturn* output = new ASTReturn();
			output->exp = this->expression();
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "return expression");

			return output;
		} else return nullptr;
	}

	ASTFunction* function () {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			auto output = new ASTFunction();
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->text();
			else expected("Identifier", "'fn' keyword");
			output->type = this->functionType();

			this->context = this->context->push();
			for (auto const &param : output->type->params)
				this->context->add(param->name, param);
			output->stms = this->statement();
			this->context = this->context->pop();

			this->context->add(output->name, output);
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
		if (!this->context->resolve())
			error("Could not resolve all names!");
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
			ASTUnop* output = new ASTUnop(Token::Type::SUB);
			output->exp = this->atom();
			return (ASTExpression*) output;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			return this->expression();
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			auto var = this->variable();
			string name;
			if (var == nullptr)
				name = string(this->lexer->text());
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				ASTCall* fnCall = this->call();
				if (var == nullptr)
					this->context->addUnresolved(name, &fnCall->var);
				else fnCall->var = var;
				return fnCall;
			} else return var;
		} else return this->constant();
	}

	ASTVariable* _var_def () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			ASTVariable* output = new ASTVariable();
			output->name = this->lexer->text();

			char* typeName = nullptr;
			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				if (this->lexer->isNextType(Token::Type::ID)) {
					output->type = this->type();
					if (output->type == nullptr)
						typeName = this->lexer->text();
				}
			}

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == nullptr)
					expected("expression", "'='");
			}

			if (output->type == nullptr) {
				if (output->expression != nullptr) {
					ASTType* ty = output->expression->getType(context);
					if (ty != nullptr) {
						output->type = ty;
					} else error("Type could not be inferred!");
				} else {
					if (typeName != nullptr)
						this->context->addUnresolved(typeName, &output->type);
				}
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
		} else output->retType = ASTPrimitiveType::_void;
		return output;
	}

	vector<ASTVariable*> functionParameters () {
		vector<ASTVariable*> output;
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			ASTVariable* fnParam = this->_var_def();
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
