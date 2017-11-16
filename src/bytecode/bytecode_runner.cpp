#include "bytecode/bytecode_runner.hpp"

#include <stdio.h>
#include <windows.h>

#include "compiler.hpp"

Ast_Note* remove_run_note (Ast_Declaration* decl) {
	if (decl->notes.size() > 0) {
		auto it = decl->notes.begin();
		while (it != decl->notes.end()) {
			if (strcmp((*it)->name, "run") == 0) {
				auto output = (*it);
				decl->notes.erase(it);
				return output;
			} else it++;
		}
	}
	return NULL;
}

void push_parameter (DCCallVM* vm, Ast_Literal* lit) {
	switch (lit->literal_type) {
		case AST_LITERAL_UNSIGNED_INT:
		case AST_LITERAL_SIGNED_INT: {
			switch (lit->inferred_type->byte_size) {
				case 1: dcArgChar(vm, lit->int_value); break;
				case 2: dcArgShort(vm, lit->int_value); break;
				case 4: dcArgInt(vm, lit->int_value); break;
				case 8: dcArgLongLong(vm, lit->int_value); break;
			}
		}
		case AST_LITERAL_DECIMAL: {
			switch (lit->inferred_type->byte_size) {
				case 4: dcArgFloat(vm, lit->decimal_value); break;
				case 8: dcArgDouble(vm, lit->decimal_value); break;
			}
		}
		case AST_LITERAL_STRING: {
			dcArgPointer(vm, lit->string_value); break;
		}
	}
}

size_t run_function (DCCallVM* vm, Ast_Function* func, Ast_Note* run_note) {
	auto ret_ty = static_cast<Ast_Type_Definition*>(func->type->return_type);
	if (func->foreign_module_name) {
		// TODO: have some notion of calling convention options
		dcMode(vm, DC_CALL_C_X64_WIN64);
		dcReset(vm);
		for (auto exp : run_note->arguments->args) {
			// TODO: passing complex expression should trigger more bytecode execution
			if (exp->exp_type == AST_EXPRESSION_LITERAL) {
				auto lit = static_cast<Ast_Literal*>(exp);
				push_parameter(vm, lit);
			} else {
				Light_Compiler::inst->error_stop(run_note, "#run can only have literal arguments!");
			}
		}
		// TODO: make platform layer to load functions from (#ifdef _WIN32)
		HMODULE module = LoadLibrary(func->foreign_module_name);
		DCpointer fn_ptr = (DCpointer) GetProcAddress(module, func->name);

		void* ret = dcCallPointer(vm, fn_ptr);

		// TEST
		DWORD written;
		LPSTR msg = "\nDYNCALL is super awesome!\n\n";
		DWORD length = strlen(msg);
		WriteFile(ret, msg, length, &written, NULL);
		FlushFileBuffers(ret);

		return reinterpret_cast<size_t>(ret);
	} else {
		for (auto inst : func->bytecode) {
			Light_Compiler::inst->interp->run(inst);
		}
		Light_Compiler::inst->interp->dump();
	}
	return NULL;
}

void Bytecode_Runner::on_statement (Ast_Statement* stm) {
	this->run(stm);
    this->to_next(stm);
}

void Bytecode_Runner::run (Ast_Statement* stm) {
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			this->run(static_cast<Ast_Block*>(stm));
			return;
		}
		case AST_STATEMENT_DECLARATION: {
			this->run(static_cast<Ast_Declaration*>(stm));
			return;
		}
		case AST_STATEMENT_RETURN: {
			this->run(static_cast<Ast_Return*>(stm));
			return;
		}
		case AST_STATEMENT_EXPRESSION: {
			this->run(static_cast<Ast_Expression*>(stm));
			return;
		}
	}
}

void Bytecode_Runner::run (Ast_Block* block) {
}

void Bytecode_Runner::run (Ast_Declaration* decl) {
	if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
		auto func = static_cast<Ast_Function*>(decl->expression);
		auto run_note = remove_run_note(decl);
		if (run_note) run_function(this->vm, func, run_note);
	}
	this->run(decl->type);
	if (decl->expression) this->run(decl->expression);
}

void Bytecode_Runner::run (Ast_Return* ret) {
}

void Bytecode_Runner::run (Ast_Expression* exp) {
	switch (exp->exp_type) {
		case AST_EXPRESSION_FUNCTION: {
			this->run(static_cast<Ast_Function*>(exp));
			return;
		}
		case AST_EXPRESSION_TYPE_DEFINITION: {
			//this->run(static_cast<Ast_Type_Definition*>(exp));
			return;
		}
		case AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS: {
			//this->run(static_cast<Ast_Comma_Separated_Arguments*>(exp));
			return;
		}
		case AST_EXPRESSION_CALL: {
			this->run(static_cast<Ast_Function_Call*>(exp));
			return;
		}
		case AST_EXPRESSION_BINARY: {
			this->run(static_cast<Ast_Binary*>(exp));
			return;
		}
		case AST_EXPRESSION_UNARY: {
			this->run(static_cast<Ast_Unary*>(exp));
			return;
		}
		case AST_EXPRESSION_IDENT: {
			this->run(static_cast<Ast_Ident*>(exp));
			return;
		}
		case AST_EXPRESSION_LITERAL: {
			this->run(static_cast<Ast_Literal*>(exp));
			return;
		}
		default: return;
	}
}

void Bytecode_Runner::run (Ast_Function* fn) {
}

void Bytecode_Runner::run (Ast_Function_Call* call) {
}

void Bytecode_Runner::run (Ast_Binary* binop) {
}

void Bytecode_Runner::run (Ast_Unary* unop) {
}

void Bytecode_Runner::run (Ast_Ident* ident) {
}

void Bytecode_Runner::run (Ast_Literal* lit) {
}
