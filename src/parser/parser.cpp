#pragma once

#include "parser/parser.hpp"
#include "parser/printer.hpp"

template <typename T>
T* set_ast_location (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}
#define AST_NEW(T, ...) set_ast_location(lexer, new T(__VA_ARGS__))

Parser::Parser (const char* filepath) {
	this->lexer = new Lexer(filepath);
	this->currentScope = AST_NEW(Ast_Block);
	this->currentScope->name = "<global>";
}

bool Parser::block () {
	Ast_Statement* stm;
	while (stm = this->statement())
		this->currentScope->list.push_back(stm);
	return true;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_EOF: {
			cout << "EOF!\n";
			exit(EXIT_FAILURE);
		}
		case TOKEN_STM_END: {
			this->lexer->skip(1);
			return NULL;
		}
		case TOKEN_RETURN: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Return);
			output->exp = this->expression();
			this->lexer->check_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip(1);
			this->scopePush("<anon>");
			this->block();
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			auto output = this->currentScope;
			this->scopePop();
			return output;
		}
		case TOKEN_ID: {
			auto ident = this->ident();
			if (ident) {
				if (this->lexer->isNextType(TOKEN_COLON)) {
					auto decl = AST_NEW(Ast_Declaration);
					decl->identifier = ident;
					this->lexer->skip(1);
					decl->type = this->type_instance();
					if (this->lexer->isNextType(TOKEN_COLON)) {
						this->lexer->skip(1);
						decl->decl_flags |= DECL_FLAG_CONSTANT;
					} else if (this->lexer->isNextType(TOKEN_EQUAL)) {
						this->lexer->skip(1);
					} else cout << "-ERROR-\n";
					decl->expression = this->expression();
					this->lexer->check_skip(TOKEN_STM_END);
					return decl;
				} else {
					auto exp = this->expression(ident);
					if (exp) this->lexer->check_skip(TOKEN_STM_END);
					return exp;
				}
			}
		}
		default: {
			auto exp = this->expression();
			if (exp) this->lexer->check_skip(TOKEN_STM_END);
			return exp;
		}
	}
}

Ast_Declaration* Parser::declaration () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		auto decl = AST_NEW(Ast_Declaration);
		decl->identifier = this->ident();

		if (this->lexer->isNextType(TOKEN_COLON)) {
			this->lexer->skip(1);
			decl->type = this->type_instance();
		}

		if (this->lexer->isNextType(TOKEN_COLON)) {
			this->lexer->skip(1);
			decl->decl_flags |= DECL_FLAG_CONSTANT;
		} else if (this->lexer->isNextType(TOKEN_EQUAL)) {
			this->lexer->skip(1);
		}

		if (!this->lexer->isNextType(TOKEN_STM_END))
			decl->expression = this->expression();

		return decl;
	} else return NULL;
}

Ast_Type_Definition* Parser::type_definition () {
	return NULL;
}

Ast_Struct_Type* Parser::structType (string name) {
	return NULL;
}

Ast_Type_Instance* Parser::type_instance () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		auto ty_inst = AST_NEW(Ast_Struct_Type);
		ty_inst->name = this->lexer->text();
		return ty_inst;
	} else if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		auto ty_inst = AST_NEW(Ast_Function_Type);
		this->lexer->skip(1);

		Ast_Type_Instance* _tmp;
		while (_tmp = this->type_instance())
			ty_inst->parameters.push_back(_tmp);
		this->lexer->check_skip(TOKEN_PAR_CLOSE);

		if (this->lexer->isNextType(TOKEN_ARROW)) {
			this->lexer->skip(1);
			ty_inst->return_type = this->type_instance();
		} else { /* TODO: set return type to void */ }

		return ty_inst;
	} else return NULL;
}

Ast_Function* Parser::function () {
	return NULL;
}

Ast_Function_Type* Parser::_functionType () {
	return NULL;
}

void Parser::_functionParameters (vector<Ast_Declaration*>* output) {
	return;
}

Ast_Expression* Parser::expression (Ast_Ident* initial, short minPrecedence) {
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
			_tmp->rhs = this->expression(NULL, nextMinPrec);
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
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->isNextType(TOKEN_FUNCTION)) {
		this->lexer->skip(1);

		auto fn = AST_NEW(Ast_Function);
		if (this->lexer->isNextType(TOKEN_ID)) {
			fn->name = this->lexer->text();
		}

		this->lexer->check_skip(TOKEN_PAR_OPEN);
		Ast_Declaration* decl;
		while (decl = this->declaration()) {
			ASTPrinter::print(decl);
			decl = this->declaration();
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);

		if (this->lexer->isNextType(TOKEN_ARROW)) {
			this->lexer->skip(1);
			ASTPrinter::print(this->type_instance());
		} else { /* TODO: set return type to void */ }

		//TODO: parse function scope (Ast_Block)

		return fn;
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
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
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
