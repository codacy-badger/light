#pragma once

#include "parser/parser.hpp"

#include "compiler.hpp"
#include "parser/printer.hpp"

template <typename T>
T* setup_ast_node (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}
#define AST_NEW(T, ...) setup_ast_node(lexer, new T(__VA_ARGS__))

Parser::Parser (Light_Compiler* compiler, const char* filepath) {
	this->compiler = compiler;
	this->lexer = new Lexer(filepath);
	this->currentScope = AST_NEW(Ast_Block);
	this->currentScope->name = "<global>";
}

bool Parser::block () {
	Ast_Statement* stm;
	while (stm = this->statement()) {

		if (stm->stm_type == AST_STATEMENT_IMPORT) {
			auto imp = static_cast<Ast_Import*>(stm);
			if (imp->import_flags & IMPORT_INCLUDE_CONTENT) {
				this->lexerPush(imp->filepath);
				if (!this->lexer->buffer->is_valid())
					Light_Compiler::report_error(imp, "File not found: '%s'", imp->filepath);
			} else {
				if (imp->import_flags & IMPORT_IS_NATIVE)
					Light_Compiler::report_warning(imp,
						"Native imports not yet supported! use 'import!'");
				else Light_Compiler::report_warning(imp,
					"Dynamic imports not yet supported! use 'import!'");
			}
		} else {
			this->currentScope->list.push_back(stm);
			if (stm->stm_type == AST_STATEMENT_DECLARATION)
				this->toNext(static_cast<Ast_Declaration*>(stm));
		}

		if (this->lexer->isNextType(TOKEN_EOF)) {
			if (this->lexer->parent) this->lexerPop();
			else return true;
		}
	}
	return false;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_EOF: this->lexer->report_unexpected();
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
		case TOKEN_IMPORT: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Import);

			if (this->lexer->optional_skip(TOKEN_EXCLAMATION))
				output->import_flags |= IMPORT_INCLUDE_CONTENT;
			else if (this->lexer->optional_skip(TOKEN_AT))
				output->import_flags |= IMPORT_IS_NATIVE;

			if (this->lexer->isNextType(TOKEN_STRING))
				output->filepath = this->lexer->text();

			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip(1);
			this->scopePush("<anon>");
			this->block();
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return this->scopePop();
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
					} else if (this->lexer->isNextType(TOKEN_STM_END)) {
						this->lexer->skip(1);
						return decl;
					}

					decl->expression = this->expression();
					if (this->lexer->isNextType(TOKEN_STM_END))
						this->lexer->skip(1);
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
		} else ty_inst->return_type = Light_Compiler::type_def_void;

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
    if (output = this->_atom(initial)) {
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

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->lexer->isNextType(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			auto fnPtr = reinterpret_cast<Ast_Function*>(output);
			return this->call(fnPtr);
		} else return output;
	} else if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->isNextType(TOKEN_FUNCTION)) {
		this->lexer->skip(1);

		auto fn = AST_NEW(Ast_Function);

		auto fn_type = AST_NEW(Ast_Function_Type);
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			this->lexer->skip(1);
			Ast_Declaration* decl;
			while (decl = this->declaration()) {
				fn_type->parameters.push_back(decl->type);
				decl = this->declaration();
			}
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
		}
		if (this->lexer->isNextType(TOKEN_ARROW)) {
			this->lexer->skip(1);
			fn_type->return_type = this->type_instance();
		} else fn_type->return_type = Light_Compiler::type_def_void;
		fn->type = fn_type;

		if (!this->lexer->isNextType(TOKEN_STM_END)) {
			this->lexer->check_skip(TOKEN_BRAC_OPEN);
			this->scopePush("<anon>");
			this->block();
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			fn->scope = this->scopePop();
		}

		return fn;
	} else if (this->lexer->isNextType(TOKEN_SUB)) {
		this->lexer->skip(1);
		auto unop = AST_NEW(AST_Unary, TOKEN_SUB);
		unop->exp = this->_atom();
		return unop;
	} else if (this->lexer->isNextType(TOKEN_ADD)) {
		this->lexer->skip(1);
		return this->expression();
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

void Parser::lexerPush (const char* filepath) {
	assert(this->lexer);
	this->lexer = new Lexer(filepath, this->lexer);
}

Lexer* Parser::lexerPop () {
	assert(this->lexer->parent);
	auto _tmp = this->lexer;
	this->lexer = this->lexer->parent;
	return _tmp;
}

void Parser::scopePush (string name) {
	assert(this->currentScope);
	this->currentScope = AST_NEW(Ast_Block, this->currentScope);
	this->currentScope->name = name;
}

Ast_Block* Parser::scopePop () {
	assert(this->currentScope->parent);
	auto _tmp = this->currentScope;
	this->currentScope = this->currentScope->parent;
	return _tmp;
}
