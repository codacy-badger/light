#include "bytecode/types.hpp"

bool Bytecode_Types::contains (Ast_Type_Definition* type_def) {
    for (auto _type_def : this->all_types) {
        if (_type_def == type_def) return true;
    }
    return false;
}

void Bytecode_Types::add (Ast_Type_Definition* type_def) {
    if (!this->contains(type_def)) {
        switch (type_def->typedef_type) {
            case AST_TYPEDEF_FUNCTION: {
                Ast_Type_Definition* _ty = NULL;
                auto func_type = static_cast<Ast_Function_Type*>(type_def);
                for (auto decl : func_type->parameter_decls) {
                    this->add(static_cast<Ast_Type_Definition*>(decl->type));
                }
                this->add(static_cast<Ast_Type_Definition*>(func_type->return_type));
                break;
            }
        	case AST_TYPEDEF_POINTER: {
                auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
                auto _ty = static_cast<Ast_Type_Definition*>(ptr_type->base);
                this->add(_ty);
                break;
            }
            default: break;
        }
        this->print(stdout, type_def);
        fprintf(stdout, "\n");
        this->all_types.push_back(type_def);
    }
}

void Bytecode_Types::print (FILE* out, Ast_Type_Definition* type_def) {
    switch (type_def->typedef_type) {
        case AST_TYPEDEF_FUNCTION: {
            auto func_type = static_cast<Ast_Function_Type*>(type_def);
            fprintf(out, "fn ( ");
            Ast_Type_Definition* _ty = NULL;
            if (func_type->parameter_decls.size() > 0) {
                _ty = static_cast<Ast_Type_Definition*>(func_type->parameter_decls[0]->type);
                this->print(out, _ty);
                for (int i = 1; i < func_type->parameter_decls.size(); i++) {
                    fprintf(out, ", ");
                    _ty = static_cast<Ast_Type_Definition*>(func_type->parameter_decls[i]->type);
                    this->print(out, _ty);
                }
            }
            fprintf(out, " ) -> ");
            _ty = static_cast<Ast_Type_Definition*>(func_type->return_type);
            this->print(out, _ty);
            break;
        }
    	case AST_TYPEDEF_STRUCT: {
            auto strc_type = static_cast<Ast_Struct_Type*>(type_def);
            fprintf(out, strc_type->name);
            break;
        }
    	case AST_TYPEDEF_POINTER: {
            auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
            fprintf(out, "*");
            auto _ty = static_cast<Ast_Type_Definition*>(ptr_type->base);
            this->print(out, _ty);
            break;
        }
    }
}
