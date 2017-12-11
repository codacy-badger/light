#include "parser/pipe/unique_types.hpp"

#include "compiler.hpp"

void Unique_Types::on_statement(Ast_Statement* stm) {
    this->unique(stm);
    this->to_next(stm);
}

void Unique_Types::unique (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK: {
            auto block = static_cast<Ast_Block*>(stm);
            for (auto _stm : block->list)
                this->unique(_stm);
            break;
        }
        case AST_STATEMENT_DECLARATION: {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->type) this->unique(&decl->type);
			if (decl->expression) this->unique(&decl->expression);
			break;
        }
        case AST_STATEMENT_IF: {
            auto _if = static_cast<Ast_If*>(stm);
            this->unique(&_if->condition);
            this->unique(_if->then_statement);
            if (_if->else_statement) this->unique(_if->else_statement);
			break;
        }
        case AST_STATEMENT_WHILE: {
            auto _while = static_cast<Ast_While*>(stm);
            this->unique(&_while->condition);
            this->unique(_while->statement);
			break;
        }
        case AST_STATEMENT_RETURN: {
            auto ret = static_cast<Ast_Return*>(stm);
            this->unique(&ret->exp);
			break;
        }
        case AST_STATEMENT_EXPRESSION: {
            auto exp = static_cast<Ast_Expression*>(stm);
            this->unique(&exp);
			break;
        }
        default: break;
    }
}

void Unique_Types::unique (Ast_Expression** exp) {
    switch ((*exp)->exp_type) {
        case AST_EXPRESSION_CAST: {
            auto cast = reinterpret_cast<Ast_Cast*>(*exp);
            this->unique(&cast->cast_to);
            break;
        }
        case AST_EXPRESSION_FUNCTION: {
            auto func = static_cast<Ast_Function*>(*exp);
            this->unique(&func->type);
			if (func->scope) this->unique(func->scope);
            break;
        }
		case AST_EXPRESSION_TYPE_DEFINITION: {
			this->unique(reinterpret_cast<Ast_Type_Definition**>(exp));
			break;
		}
		case AST_EXPRESSION_BINARY: {
            auto binary = static_cast<Ast_Binary*>(*exp);
			this->unique(&binary->lhs);
			this->unique(&binary->rhs);
			break;
		}
		case AST_EXPRESSION_UNARY: {
            auto unary = static_cast<Ast_Unary*>(*exp);
			this->unique(&unary->exp);
			break;
		}
        default: break;
    }
}

void Unique_Types::unique (Ast_Type_Definition** type_def) {
    switch ((*type_def)->typedef_type) {
        case AST_TYPEDEF_STRUCT: {
            this->unique(reinterpret_cast<Ast_Struct_Type**>(type_def));
            break;
        }
        case AST_TYPEDEF_POINTER: {
            this->unique(reinterpret_cast<Ast_Pointer_Type**>(type_def));
            break;
        }
        case AST_TYPEDEF_ARRAY: {
            this->unique(reinterpret_cast<Ast_Array_Type**>(type_def));
            break;
        }
        case AST_TYPEDEF_FUNCTION: {
            this->unique(reinterpret_cast<Ast_Function_Type**>(type_def));
            break;
        }
        default: break;
    }
}

void Unique_Types::unique (Ast_Struct_Type** _struct) {
    (*_struct) = Light_Compiler::inst->types->get_unique_struct_type(*_struct);
    for (int i = 0; i < (*_struct)->attributes.size(); i++) {
        this->unique((*_struct)->attributes[i]);
    }
}

void Unique_Types::unique (Ast_Pointer_Type** ptr_type) {
    this->unique(&(*ptr_type)->base);
    (*ptr_type) = Light_Compiler::inst->types->get_unique_pointer_type(*ptr_type);
    Light_Compiler::inst->types->compute_type_name_if_needed(*ptr_type);
}

void Unique_Types::unique (Ast_Array_Type** arr_type) {
    this->unique(&(*arr_type)->base);
    (*arr_type) = Light_Compiler::inst->types->get_unique_array_type(*arr_type);
    Light_Compiler::inst->types->compute_type_name_if_needed(*arr_type);
}

void Unique_Types::unique (Ast_Function_Type** func_type) {
    for (int i = 0; i < (*func_type)->parameter_decls.size(); i++) {
        this->unique((*func_type)->parameter_decls[i]);
    }
    this->unique(&(*func_type)->return_type);

    (*func_type) = Light_Compiler::inst->types->get_unique_function_type(*func_type);
    Light_Compiler::inst->types->compute_type_name_if_needed(*func_type);
}
