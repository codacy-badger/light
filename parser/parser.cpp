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

#include "ast/ast.hpp"

using namespace std;

#define AST_NEW(T, ...) setASTLocation(lexer, new T(__VA_ARGS__))
template <typename T>
T* setASTLocation (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}

struct Parser {
	Lexer* lexer;
	ASTScope* currentScope;

	template <typename LexerParam>
	Parser (LexerParam param) {
		this->lexer = new Lexer(param);
		this->currentScope = AST_NEW(ASTScope, "<global>");
		this->currentScope->add("void", ASTPrimitiveType::_void);
		this->currentScope->add("i1",   ASTPrimitiveType::_i1);
		this->currentScope->add("i8",   ASTPrimitiveType::_i8);
		this->currentScope->add("i16",  ASTPrimitiveType::_i16);
		this->currentScope->add("i32",  ASTPrimitiveType::_i32);
		this->currentScope->add("i64",  ASTPrimitiveType::_i64);
		this->currentScope->add("i128", ASTPrimitiveType::_i128);
	}

	ASTScope* program () {
		this->statements();
		return this->currentScope;
	}

	bool statements () {
		ASTStatement* exp;
		while (this->statement(&exp)) {
			this->currentScope->list.push_back(exp);
		}
		return true;
	}

	bool statement (ASTStatement** output) {
		switch (this->lexer->nextType) {
			case Token::Type::STM_END: {
				this->lexer->skip(1);
				return false;
			}
			case Token::Type::TYPE: {
				return this->type(reinterpret_cast<ASTType**>(output));
			}
			case Token::Type::FUNCTION: {
				return this->function(reinterpret_cast<ASTFunction**>(output));
			}
			case Token::Type::LET: {
				return this->var_def(reinterpret_cast<ASTVariable**>(output));
			}
			case Token::Type::RETURN: {
				return this->returnStm(reinterpret_cast<ASTReturn**>(output));
			}
			case Token::Type::BRAC_OPEN: {
				this->scopePush("<anon>");
				this->lexer->skip(1);
				auto result = this->statements();
				if(this->lexer->isNextType(Token::Type::BRAC_CLOSE))
					this->lexer->skip(1);
				else expected("'}'", "statements");
				(*output) = this->currentScope;
				this->scopePop();
				return result;
			}
			default: {
				if (this->expression(reinterpret_cast<ASTExpression**>(output))) {
					if (this->lexer->isNextType(Token::Type::STM_END)) {
						this->lexer->skip(1);
						return true;
					} else expected("';'", "expression");
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
				this->_typeInstance(&ty);
				if (ty != nullptr) this->currentScope->add(name, ty);
				if (this->lexer->isNextType(Token::Type::STM_END))
					this->lexer->skip(1);
				else expected("';'", "type alias");
			} else {
				auto ptr = reinterpret_cast<ASTStructType**>(output);
				return this->structType(ptr, name);
			}

			return true;
		} else return false;
	}

	bool structType (ASTStructType** output, string name) {
		(*output) = AST_NEW(ASTStructType, name);

		ASTStatement* stm;
		if (this->lexer->isNextType(Token::Type::BRAC_OPEN))
			this->lexer->skip(1);
		else expected("'{'", "type name");
		while (this->_typeBody(&stm)) {
			if (auto var = dynamic_cast<ASTVariable*>(stm)) {
				(*output)->attrs.push_back(var);
				if (this->lexer->isNextType(Token::Type::STM_END))
					this->lexer->skip(1);
				else expected("';'", "type attribute");
			} else if (auto fn = dynamic_cast<ASTFunction*>(stm))
				(*output)->methods.push_back(fn);
			else error("Expected attribute or method inside type");
		}
		if (this->lexer->isNextType(Token::Type::BRAC_CLOSE))
			this->lexer->skip(1);
		else expected("'}'", "type body");

		this->currentScope->add((*output)->name, (*output));
		return true;
	}

	bool _typeBody (ASTStatement** output) {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			return this->function(reinterpret_cast<ASTFunction**>(output));
		} else if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);
			return this->_var_def(reinterpret_cast<ASTVariable**>(output));
		} else return this->_var_def(reinterpret_cast<ASTVariable**>(output));
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
				this->statements();
				if(this->lexer->isNextType(Token::Type::BRAC_CLOSE))
					this->lexer->skip(1);
				else expected("'}'", "function body");
				(*output)->stm = this->currentScope;
				this->scopePop();
			}
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
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "variable declaration");

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
			auto typeExp = this->currentScope->get(typeName);
			if (typeExp == nullptr) {
				this->currentScope->addUnresolved(typeName, output);
			} else {
				(*output) = dynamic_cast<ASTType*>(typeExp);
				if ((*output) == nullptr) error("Name is not a type!");
			}
			return true;
		} else return false;
	}

	bool returnStm (ASTReturn** output) {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			(*output) = AST_NEW(ASTReturn);
			this->expression(&(*output)->exp);
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "return expression");
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
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else error("Expected closing parenthesys after expression");
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
			auto memAsExp = reinterpret_cast<ASTExpression**>(&deref->memory);
			auto result = this->_atom(memAsExp);
			(*output) = deref;
			return result;
		} else if (this->lexer->isNextType(Token::Type::MUL)) {
			this->lexer->skip(1);
			auto ref = AST_NEW(ASTRef);
			auto memAsExp = reinterpret_cast<ASTExpression**>(&ref->memory);
			auto result = this->_atom(memAsExp);
			(*output) = ref;
			return result;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			this->expression(output);
			return true;
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			this->variable(output);
			string name;
			if ((*output) == nullptr)
				name = string(this->lexer->text());
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				ASTCall* fnCall;
				this->call(&fnCall);
				if ((*output) == nullptr)
					this->currentScope->addUnresolved(name, &fnCall->var);
				else fnCall->var = dynamic_cast<ASTFunction*>(*output);
				(*output) = fnCall;
				return true;
			} else return true;
		} else return this->literal(reinterpret_cast<ASTLiteral**>(output));
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

	bool call (ASTCall** output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			(*output) = AST_NEW(ASTCall);
			ASTExpression* exp;
			while (this->expression(&exp)) {
				(*output)->params.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "call arguments");

			return true;
		} else return false;
	}

	bool variable (ASTExpression** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			string name(this->lexer->nextText);
			(*output) = this->currentScope->get(name);
			if ((*output) != nullptr) this->lexer->skip(1);

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
		this->currentScope = AST_NEW(ASTScope, name, this->currentScope);
	}

	void scopePop () {
		assert(this->currentScope->parent);
		auto pUnr = &this->currentScope->parent->unresolved;
		for (auto const &it : this->currentScope->unresolved) {
			auto entry = pUnr->find(it.first);
			if (entry == pUnr->end()) {
				(*pUnr)[it.first] = it.second;
			} else {
				for (auto const &ref : it.second)
					(*pUnr)[it.first].push_back(ref);
			}
		}
		this->currentScope = this->currentScope->parent;
	}

	void error (const char* message) {
		cout << "[Light] ERROR: " << message << endl;
		cout << "        at ";
		this->lexer->buffer->printLocation();
		cout << endl;
		exit(EXIT_FAILURE);
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
