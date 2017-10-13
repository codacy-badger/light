#pragma once

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "buffer.cpp"

#include "lexer/lexer.cpp"

#include "pipes.cpp"
#include "ast.hpp"

using namespace std;

#define CHECK_TYPE(T, S) if(this->lexer->isNextType(Token::Type::T))	\
		this->lexer->skip(1);											\
	else expected(Token::typeToString(Token::Type::T), S)

#define AST_NEW(T, ...) setASTLocation(lexer, new T(__VA_ARGS__))
template <typename T>
T* setASTLocation (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}

template <typename T, typename O>
T* cast2 (O* obj) {
	return reinterpret_cast<T*>(obj);
}

struct Parser : Pipe {
	Lexer* lexer;
	ASTBlock* currentScope;

	template <typename LexerParam>
	Parser (LexerParam param) {
		this->lexer = new Lexer(param);
		this->currentScope = AST_NEW(ASTBlock, "<global>");
		this->currentScope->add("void", ASTPrimitiveType::_void);
		this->currentScope->add("i1",   ASTPrimitiveType::_i1);
		this->currentScope->add("i8",   ASTPrimitiveType::_i8);
		this->currentScope->add("i16",  ASTPrimitiveType::_i16);
		this->currentScope->add("i32",  ASTPrimitiveType::_i32);
		this->currentScope->add("i64",  ASTPrimitiveType::_i64);
		this->currentScope->add("i128", ASTPrimitiveType::_i128);
	}

	bool block () {
		ASTStatement* exp;
		while (exp = this->statement())
			this->currentScope->list.push_back(exp);
		return true;
	}

	ASTStatement* statement () {
		switch (this->lexer->nextType) {
			case Token::Type::STM_END: {
				this->lexer->skip(1);
				return nullptr;
			}
			case Token::Type::TYPE: {
				return this->type();
			}
			case Token::Type::FUNCTION: {
				return this->function();
			}
			case Token::Type::LET: {
				return this->var_def();
			}
			case Token::Type::RETURN: {
				return this->returnStm();
			}
			case Token::Type::BRAC_OPEN: {
				this->lexer->skip(1);
				this->scopePush("<anon>");
				this->block();
				CHECK_TYPE(BRAC_CLOSE, "block");
				auto output = this->currentScope;
				this->scopePop();
				return output;
			}
			default: {
				auto exp = this->expression();
				if (exp) CHECK_TYPE(STM_END, "expression");
				return exp;
			}
		}
	}

	ASTType* type () {
		if (this->lexer->isNextType(Token::Type::TYPE)) {
			this->lexer->skip(1);

			auto name = this->lexer->text();
			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				auto result = this->_typeInstance();
				this->currentScope->add(name, result);
				CHECK_TYPE(STM_END, "type alias");
				return result;
			} else if (this->lexer->isNextType(Token::Type::BRAC_OPEN)) {
				auto result = this->structType(name);
				this->toNext(result);
				return result;
			}
		}
		return nullptr;
	}

	ASTStructType* structType (string name) {
		auto output = AST_NEW(ASTStructType, name);
		this->currentScope->add(name, output);

		ASTStatement* stm = nullptr;
		CHECK_TYPE(BRAC_OPEN, "type name");
		while (stm = this->_typeBody()) {
			if (auto var = dynamic_cast<ASTVariable*>(stm)) {
				output->attrs.push_back(var);
				CHECK_TYPE(STM_END, "type attribute");
			} else if (auto fn = dynamic_cast<ASTFunction*>(stm))
				output->methods.push_back(fn);
			else expected("attribute or method", "type");
		}
		CHECK_TYPE(BRAC_CLOSE, "type body");

		return output;
	}

	ASTStatement* _typeBody () {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			return this->function();
		} else if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);
			return this->_var_def();
		} else return this->_var_def();
	}

	ASTFunction* function () {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			auto output = AST_NEW(ASTFunction);
			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->text();
			else expected("Identifier", "'fn' keyword");
			output->type = this->_functionType();
			this->currentScope->add(output->name, output);

			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else if (this->lexer->isNextType(Token::Type::BRAC_OPEN)) {
				this->scopePush(output->name);
				this->lexer->skip(1);
				for (auto const &param : output->type->params)
					this->currentScope->add(param->name, param);
				this->block();
				CHECK_TYPE(BRAC_CLOSE, "function body");
				output->stm = this->currentScope;
				this->scopePop();
			}

			this->toNext(output);
			return output;
		} else return nullptr;
	}

	ASTFnType* _functionType () {
		auto output = AST_NEW(ASTFnType);
		this->_functionParameters(&output->params);
		if (this->lexer->isNextType(Token::Type::ARROW)) {
			this->lexer->skip(1);
			output->retType = this->_typeInstance();
		} else output->retType = ASTPrimitiveType::_void;
		return output;
	}

	void _functionParameters (vector<ASTVariable*>* output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);

			ASTVariable* fnParam = nullptr;
			while (fnParam = this->_var_def()) {
				output->push_back(fnParam);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}

			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "function parameters");
		}
	}

	ASTVariable* var_def () {
		if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);

			auto output = this->_var_def();
			CHECK_TYPE(STM_END, "variable declaration");

			this->currentScope->add(output->name, output);
			return output;
		} else return nullptr;
	}

	ASTVariable* _var_def () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			auto output = AST_NEW(ASTVariable);
			output->name = this->lexer->text();

			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				output->type = this->_typeInstance();
			}

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == nullptr)
					expected("expression", "'='");
			}
			return output;
		} else return nullptr;
	}

	ASTType* _typeInstance () {
		if (this->lexer->isNextType(Token::Type::MUL)) {
			this->lexer->skip(1);
			auto ptrTy = AST_NEW(ASTPointerType);
			ptrTy->base = this->_typeInstance();
			return ptrTy;
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->text();
			auto output = this->currentScope->get<ASTType>(typeName);
			if (!output) output = AST_NEW(ASTUnresolvedTy, typeName);
			return output;
		} else return nullptr;
	}

	ASTReturn* returnStm () {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			auto output = AST_NEW(ASTReturn);
			output->exp = this->expression();
			CHECK_TYPE(STM_END, "return expression");
			return output;
		} else return nullptr;
	}

	ASTExpression* expression (short minPrecedence = 1) {
		ASTExpression* output;
	    if (output = this->_atom()) {
	        Token::Type tt = this->lexer->nextType;
			auto precedence = ASTBinop::getPrecedence(tt);
			while (precedence >= minPrecedence) {
				this->lexer->skip(1);

				int nextMinPrec = precedence;
				if (ASTBinop::getLeftAssociativity(tt))
					nextMinPrec += 1;

				ASTBinop* _tmp = AST_NEW(ASTBinop, tt);
				_tmp->rhs = this->expression(nextMinPrec);
				_tmp->lhs = output;
				output = _tmp;

				tt = this->lexer->nextType;
				precedence = ASTBinop::getPrecedence(tt);
			}
	    }
		return output;
	}

	ASTExpression* _atom () {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			auto result = this->expression();
			CHECK_TYPE(PAR_CLOSE, "expression");
			return result;
		} else if (this->lexer->isNextType(Token::Type::SUB)) {
			this->lexer->skip(1);
			auto unop = AST_NEW(ASTUnop, Token::Type::SUB);
			unop->exp = this->_atom();
			return unop;
		} else if (this->lexer->isNextType(Token::Type::AMP)) {
			this->lexer->skip(1);
			auto deref = AST_NEW(ASTDeref);
			auto exp = this->_atom();
			if (auto mem = dynamic_cast<ASTMemory*>(exp)) {
				deref->memory = mem;
			} // TODO: we should report a parsing error in else block
			return deref;
		} else if (this->lexer->isNextType(Token::Type::MUL)) {
			this->lexer->skip(1);
			auto ref = AST_NEW(ASTRef);
			auto exp = this->_atom();
			if (auto mem = dynamic_cast<ASTMemory*>(exp)) {
				ref->memory = mem;
			} // TODO: we should report a parsing error in else block
			return ref;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			return this->expression();
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			auto output = this->variable();
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				auto fnPtr = reinterpret_cast<ASTFunction*>(output);
				return this->call(fnPtr);
			} else return output;
		} else return this->literal();
	}

	ASTLiteral* literal () {
		ASTLiteral* output = nullptr;
		if (this->lexer->isNextType(Token::Type::STRING)) {
			output = AST_NEW(ASTLiteral, ASTLiteral::TYPE::STRING);
			output->stringValue = this->lexer->text();
		} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
			output = AST_NEW(ASTLiteral, ASTLiteral::TYPE::INT);
			output->intValue = atoi(this->lexer->text());
		}
		return output;
	}

	ASTCall* call (ASTExpression* callee) {
		ASTCall* output = nullptr;
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			output = AST_NEW(ASTCall);
			output->fn = callee;
			ASTExpression* exp = nullptr;
			while (exp = this->expression()) {
				output->params.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}
			CHECK_TYPE(PAR_CLOSE, "function arguments");
		}
		return output;
	}

	ASTExpression* variable () {
		if (this->lexer->isNextType(Token::Type::ID)) {
			string name(this->lexer->nextText);
			auto output = this->currentScope->get(name);
			if (!output) output = AST_NEW(ASTUnresolvedExp, name);
			this->lexer->skip(1);

			Token::Type tt = this->lexer->nextType;
			while (tt == Token::Type::DOT) {
				this->lexer->skip(1);
				if (tt == Token::Type::DOT) {
					auto attr = AST_NEW(ASTAttr, output);
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

	void scopePush (std::string name) {
		assert(this->currentScope);
		this->currentScope = AST_NEW(ASTBlock, name, this->currentScope);
	}

	void scopePop () {
		assert(this->currentScope->parent);
		this->currentScope = this->currentScope->parent;
	}

	void expected (const char* expect, const char* after) {
		cout << "[Light] ERROR: Expected " << expect
			<< " after " << after << endl;
		cout << "        at ";
		this->lexer->buffer->printLocation();
		cout << endl;
		exit(EXIT_FAILURE);
	}
};
