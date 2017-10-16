#pragma once

#include "parser/parser.hpp"

#define CHECK_TYPE(T, S) if(this->lexer->isNextType(TOKEN_##T))	\
		this->lexer->skip(1);											\
	else expected("<token>", S)

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

Parser::Parser (const char* filepath) {
	this->lexer = new Lexer(filepath);
	this->currentScope = AST_NEW(Ast_Block);
	this->currentScope->name = "<global>";
}

bool Parser::block () {
	Ast_Statement* exp;
	while (exp = this->statement())
		this->currentScope->list.push_back(exp);
	return true;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_STM_END: {
			this->lexer->skip(1);
			return NULL;
		}
		case TOKEN_TYPE: {
			return this->type();
		}
		case TOKEN_FUNCTION: {
			return this->function();
		}
		case TOKEN_RETURN: {
			return this->returnStm();
		}
		case TOKEN_BRAC_OPEN: {
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

Ast_Type_Definition* Parser::type () {
	if (this->lexer->isNextType(TOKEN_TYPE)) {
		this->lexer->skip(1);

		auto name = this->lexer->text();
		if (this->lexer->isNextType(TOKEN_EQUAL)) {
			this->lexer->skip(1);
			auto result = this->_typeInstance();
			CHECK_TYPE(STM_END, "type alias");
			return result;
		} else if (this->lexer->isNextType(TOKEN_BRAC_OPEN)) {
			auto result = this->structType(name);
			return result;
		}
	}
	return NULL;
}

Ast_Struct_Type* Parser::structType (string name) {
	auto output = AST_NEW(Ast_Struct_Type, name);

	CHECK_TYPE(BRAC_OPEN, "type name");
	this->_typeBody(&output->attributes);
	CHECK_TYPE(BRAC_CLOSE, "type body");

	return output;
}

void Parser::_typeBody (vector<Ast_Declaration*>* attributes) {
	assert(!"IMPLEMENT");
}

Ast_Function* Parser::function () {
	if (this->lexer->isNextType(TOKEN_FUNCTION)) {
		this->lexer->skip(1);
		auto output = AST_NEW(Ast_Function);
		if (this->lexer->isNextType(TOKEN_ID))
			output->name = this->lexer->text();
		else expected("Identifier", "'fn' keyword");
		output->type = this->_functionType();

		if (this->lexer->isNextType(TOKEN_STM_END))
			this->lexer->skip(1);
		else if (this->lexer->isNextType(TOKEN_BRAC_OPEN)) {
			this->scopePush(output->name);
			this->lexer->skip(1);
			this->block();
			CHECK_TYPE(BRAC_CLOSE, "function body");
			output->stm = this->currentScope;
			this->scopePop();
		}
		return output;
	} else return NULL;
}

Ast_Function_Type* Parser::_functionType () {
	auto output = AST_NEW(Ast_Function_Type);
	this->_functionParameters(&output->parameters);
	if (this->lexer->isNextType(TOKEN_ARROW)) {
		this->lexer->skip(1);
		output->retType = this->_typeInstance();
	} else output->retType = Ast_Primitive_Type::_void;
	return output;
}

void Parser::_functionParameters (vector<Ast_Declaration*>* output) {
	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);

		/*Ast_Declaration* fnParam = NULL;
		while (fnParam = this->_var_def()) {
			output->push_back(fnParam);
			if (this->lexer->isNextType(TOKEN_COMMA))
				this->lexer->skip(1);
		}*/

		if(this->lexer->isNextType(TOKEN_PAR_CLOSE))
			this->lexer->skip(1);
		else expected("closing parenthesys", "function parameters");
	}
}

Ast_Type_Definition* Parser::_typeInstance () {
	if (this->lexer->isNextType(TOKEN_MUL)) {
		this->lexer->skip(1);
		auto ptrTy = AST_NEW(Ast_Pointer_Type);
		ptrTy->base = this->_typeInstance();
		return ptrTy;
	} else if (this->lexer->isNextType(TOKEN_ID)) {
		return NULL;
	} else return NULL;
}

Ast_Return* Parser::returnStm () {
	if (this->lexer->isNextType(TOKEN_RETURN)) {
		this->lexer->skip(1);
		auto output = AST_NEW(Ast_Return);
		output->exp = this->expression();
		CHECK_TYPE(STM_END, "return expression");
		return output;
	} else return NULL;
}

Ast_Expression* Parser::expression (short minPrecedence) {
	Ast_Expression* output;
    if (output = this->_atom()) {
        Token_Type tt = this->lexer->nextType;
		auto precedence = AST_Binary::getPrecedence(tt);
		while (precedence >= minPrecedence) {
			this->lexer->skip(1);

			int nextMinPrec = precedence;
			if (AST_Binary::getLeftAssociativity(tt))
				nextMinPrec += 1;

			AST_Binary* _tmp = AST_NEW(AST_Binary, tt);
			_tmp->rhs = this->expression(nextMinPrec);
			_tmp->lhs = output;
			output = _tmp;

			tt = this->lexer->nextType;
			precedence = AST_Binary::getPrecedence(tt);
		}
    }
	return output;
}

Ast_Expression* Parser::_atom () {
	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		auto result = this->expression();
		CHECK_TYPE(PAR_CLOSE, "expression");
		return result;
	} else if (this->lexer->isNextType(TOKEN_SUB)) {
		this->lexer->skip(1);
		auto unop = AST_NEW(AST_Unary, TOKEN_SUB);
		unop->exp = this->_atom();
		return unop;
	} else if (this->lexer->isNextType(TOKEN_ADD)) {
		this->lexer->skip(1);
		return this->expression();
	} else if (this->lexer->isNextType(TOKEN_ID)) {
		auto output = this->ident();
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			auto fnPtr = reinterpret_cast<Ast_Function*>(output);
			return this->call(fnPtr);
		} else return output;
	} else return this->literal();
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->lexer->nextType) {
		case TOKEN_STRING: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_STRING;
			output->string_value = this->lexer->text();
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_INTEGER;
			output->integer_value = atoi(this->lexer->text());
			break;
		}
		default: break;
	}
	return output;
}

Ast_Function_Call* Parser::call (Ast_Expression* callee) {
	Ast_Function_Call* output = NULL;
	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		output = AST_NEW(Ast_Function_Call);
		output->fn = callee;
		Ast_Expression* exp = NULL;
		while (exp = this->expression()) {
			output->parameters.push_back(exp);
			if (this->lexer->isNextType(TOKEN_COMMA))
				this->lexer->skip(1);
		}
		CHECK_TYPE(PAR_CLOSE, "function arguments");
	}
	return output;
}

Ast_Ident* Parser::ident () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		Ast_Ident* output = AST_NEW(Ast_Ident);
		output->name = this->lexer->text();
		return output;
	} else return NULL;
}

void Parser::scopePush (string name) {
	assert(this->currentScope);
	this->currentScope = AST_NEW(Ast_Block, this->currentScope);
	this->currentScope->name = name;
}

void Parser::scopePop () {
	assert(this->currentScope->parent);
	this->currentScope = this->currentScope->parent;
}

void Parser::expected (const char* expect, const char* after) {
	cout << "[Light] ERROR: Expected " << expect
		<< " after " << after << endl;
	cout << "        at ";
	this->lexer->buffer->printLocation();
	cout << endl;
	exit(EXIT_FAILURE);
}
