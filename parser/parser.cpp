#pragma once

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "buffer/buffer.cpp"
#include "buffer/file_buffer.cpp"

#include "lexer/lexer.cpp"

#include "pipes.cpp"
#include "ast/ast.hpp"

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
T** cast2 (O** obj) {
	return reinterpret_cast<T**>(obj);
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
		while (this->statement(&exp))
			this->currentScope->list.push_back(exp);
		return true;
	}

	bool statement (ASTStatement** output) {
		switch (this->lexer->nextType) {
			case Token::Type::STM_END: {
				this->lexer->skip(1);
				return false;
			}
			case Token::Type::TYPE: {
				return this->type(cast2<ASTType>(output));
			}
			case Token::Type::FUNCTION: {
				return this->function(cast2<ASTFunction>(output));
			}
			case Token::Type::LET: {
				return this->var_def(cast2<ASTVariable>(output));
			}
			case Token::Type::RETURN: {
				return this->returnStm(cast2<ASTReturn>(output));
			}
			case Token::Type::BRAC_OPEN: {
				this->lexer->skip(1);
				this->scopePush("<anon>");
				auto result = this->block();
				CHECK_TYPE(BRAC_CLOSE, "block");
				(*output) = this->currentScope;
				this->scopePop();
				return result;
			}
			default: {
				if (this->expression(cast2<ASTExpression>(output))) {
					CHECK_TYPE(STM_END, "expression");
					return true;
				}
				return false;
			}
		}
	}

	bool type (ASTType** output) {
		if (this->lexer->isNextType(Token::Type::TYPE)) {
			this->lexer->skip(1);

			auto name = this->lexer->text();
			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				ASTType* ty;
				auto result = this->_typeInstance(&ty);
				this->currentScope->add(name, ty);
				CHECK_TYPE(STM_END, "type alias");
				return result;
			} else if (this->lexer->isNextType(Token::Type::BRAC_OPEN)) {
				auto ptr = cast2<ASTStructType>(output);
				auto result = this->structType(ptr, name);
				this->currentScope->add(name, (*ptr));
				this->toNext(*ptr);
				return result;
			}
		}
		return false;
	}

	bool structType (ASTStructType** output, string name) {
		(*output) = AST_NEW(ASTStructType, name);

		ASTStatement* stm;
		CHECK_TYPE(BRAC_OPEN, "type name");
		while (this->_typeBody(&stm)) {
			if (auto var = dynamic_cast<ASTVariable*>(stm)) {
				(*output)->attrs.push_back(var);
				CHECK_TYPE(STM_END, "type attribute");
			} else if (auto fn = dynamic_cast<ASTFunction*>(stm))
				(*output)->methods.push_back(fn);
			else expected("attribute or method", "type");
		}
		CHECK_TYPE(BRAC_CLOSE, "type body");

		return true;
	}

	bool _typeBody (ASTStatement** output) {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			return this->function(cast2<ASTFunction>(output));
		} else if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);
			return this->_var_def(cast2<ASTVariable>(output));
		} else return this->_var_def(cast2<ASTVariable>(output));
	}

	bool function (ASTFunction** output) {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			(*output) = AST_NEW(ASTFunction);
			if (this->lexer->isNextType(Token::Type::ID))
				(*output)->name = this->lexer->text();
			else expected("Identifier", "'fn' keyword");
			this->_functionType(&(*output)->type);
			this->currentScope->add((*output)->name, (*output));

			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else if (this->lexer->isNextType(Token::Type::BRAC_OPEN)) {
				this->scopePush((*output)->name);
				this->lexer->skip(1);
				for (auto const &param : (*output)->type->params)
					this->currentScope->add(param->name, param);
				this->block();
				CHECK_TYPE(BRAC_CLOSE, "function body");
				(*output)->stm = this->currentScope;
				this->scopePop();
			}

			this->toNext((*output));
			return true;
		} else return false;
	}

	bool _functionType (ASTFnType** output) {
		(*output) = AST_NEW(ASTFnType);
		this->_functionParameters(&(*output)->params);
		if (this->lexer->isNextType(Token::Type::ARROW)) {
			this->lexer->skip(1);
			this->_typeInstance(&(*output)->retType);
		} else (*output)->retType = ASTPrimitiveType::_void;
		return true;
	}

	bool _functionParameters (vector<ASTVariable*>* output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);

			ASTVariable* fnParam;
			while (this->_var_def(&fnParam)) {
				output->push_back(fnParam);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}

			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "function parameters");
			return true;
		} else return false;
	}

	bool var_def (ASTVariable** output) {
		if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);

			this->_var_def(output);
			CHECK_TYPE(STM_END, "variable declaration");

			this->currentScope->add((*output)->name, (*output));
			return true;
		} else return false;
	}

	bool _var_def (ASTVariable** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			(*output) = AST_NEW(ASTVariable);
			(*output)->name = this->lexer->text();

			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				this->_typeInstance(&(*output)->type);
			}

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				this->expression(&(*output)->expression);
				if ((*output)->expression == nullptr)
					expected("expression", "'='");
			}
			return output;
		} else return nullptr;
	}

	bool _typeInstance (ASTType** output) {
		if (this->lexer->isNextType(Token::Type::MUL)) {
			this->lexer->skip(1);
			auto ptrTy = AST_NEW(ASTPointerType);
			auto result = this->_typeInstance(&ptrTy->base);
			(*output) = ptrTy;
			return result;
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->text();
			(*output) = this->currentScope->get<ASTType>(typeName);
			if (!(*output)) (*output) = AST_NEW(ASTUnresolvedTy, typeName);
			return true;
		} else return false;
	}

	bool returnStm (ASTReturn** output) {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			(*output) = AST_NEW(ASTReturn);
			this->expression(&(*output)->exp);
			CHECK_TYPE(STM_END, "return expression");
			return true;
		} else return false;
	}

	bool expression (ASTExpression** output, short minPrecedence = 1) {
	    if (this->_atom(output)) {
	        Token::Type tt = this->lexer->nextType;
			auto precedence = ASTBinop::getPrecedence(tt);
			while (precedence >= minPrecedence) {
				this->lexer->skip(1);

				int nextMinPrec = precedence;
				if (ASTBinop::getLeftAssociativity(tt))
					nextMinPrec += 1;

				ASTBinop* _tmp = AST_NEW(ASTBinop, tt);
				this->expression(&_tmp->rhs, nextMinPrec);
				_tmp->lhs = (*output);
				(*output) = _tmp;

				tt = this->lexer->nextType;
				precedence = ASTBinop::getPrecedence(tt);
			}
	        return true;
	    } else return false;
	}

	bool _atom (ASTExpression** output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			auto result = this->expression(output);
			CHECK_TYPE(PAR_CLOSE, "expression");
			return result;
		} else if (this->lexer->isNextType(Token::Type::SUB)) {
			this->lexer->skip(1);
			auto unop = AST_NEW(ASTUnop, Token::Type::SUB);
			auto result = this->_atom(&unop->exp);
			(*output) = unop;
			return result;
		} else if (this->lexer->isNextType(Token::Type::AMP)) {
			this->lexer->skip(1);
			auto deref = AST_NEW(ASTDeref);
			auto result = this->_atom(cast2<ASTExpression>(&deref->memory));
			(*output) = deref;
			return result;
		} else if (this->lexer->isNextType(Token::Type::MUL)) {
			this->lexer->skip(1);
			auto ref = AST_NEW(ASTRef);
			auto result = this->_atom(cast2<ASTExpression>(&ref->memory));
			(*output) = ref;
			return result;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			this->expression(output);
			return true;
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			this->variable(output);
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				auto callPtr = cast2<ASTCall>(output);
				auto fnPtr = reinterpret_cast<ASTFunction*>(*output);
				return this->call(callPtr, fnPtr);
			} else return true;
		} else return this->literal(cast2<ASTLiteral>(output));
	}

	bool literal (ASTLiteral** output) {
		if (this->lexer->isNextType(Token::Type::STRING)) {
			(*output) = AST_NEW(ASTLiteral, ASTLiteral::TYPE::STRING);
			(*output)->stringValue = this->lexer->text();
			return true;
		} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
			(*output) = AST_NEW(ASTLiteral, ASTLiteral::TYPE::INT);
			(*output)->intValue = atoi(this->lexer->text());
			return true;
		} else return nullptr;
	}

	bool call (ASTCall** output, ASTExpression* callee) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			(*output) = AST_NEW(ASTCall);
			(*output)->fn = callee;
			ASTExpression* exp;
			while (this->expression(&exp)) {
				(*output)->params.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}
			CHECK_TYPE(PAR_CLOSE, "function arguments");

			return true;
		} else return false;
	}

	bool variable (ASTExpression** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			string name(this->lexer->nextText);
			(*output) = this->currentScope->get(name);
			if (!(*output)) (*output) = AST_NEW(ASTUnresolvedExp, name);
			this->lexer->skip(1);

			Token::Type tt = this->lexer->nextType;
			while (tt == Token::Type::DOT) {
				this->lexer->skip(1);
				if (tt == Token::Type::DOT) {
					auto attr = AST_NEW(ASTAttr, (*output));
					if (this->lexer->isNextType(Token::Type::ID))
						attr->name = this->lexer->text();
					else expected("name", "attribute access");
					(*output) = attr;
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
