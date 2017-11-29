#pragma once

#include <string.h>
#include <vector>

#include "parser/ast.hpp"

using namespace std;

struct cmp_str_types {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Types {
    map<char*, Ast_Struct_Type*, cmp_str_types> struct_types;
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    vector<Ast_Function_Type*> func_types;

    Ast_Type_Definition* get_unique_type (Ast_Type_Definition* type_def) {
        switch (type_def->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                auto _struct = static_cast<Ast_Struct_Type*>(type_def);
                return this->get_unique_struct_type(_struct);
            }
            case AST_TYPEDEF_POINTER: {
                auto _ptr = static_cast<Ast_Pointer_Type*>(type_def);
                return this->get_unique_pointer_type(_ptr);
            }
            case AST_TYPEDEF_FUNCTION: {
                auto _func = static_cast<Ast_Function_Type*>(type_def);
                return this->get_unique_function_type(_func);
            }
            default: return NULL;
        }
    }

    Ast_Struct_Type* get_unique_struct_type (Ast_Struct_Type* _struct) {
        auto it = this->struct_types.find(_struct->name);
        if (it != this->struct_types.end()) {
    		if (_struct != it->second) {
    			delete _struct;
    			return it->second;
    		} else return _struct;
        } else {
            this->struct_types[_struct->name] = _struct;
            return _struct;
        }
    }

    Ast_Pointer_Type* get_unique_pointer_type (Ast_Pointer_Type* ptr_type) {
        auto it = this->ptr_types.find(ptr_type->base);
        if (it != this->ptr_types.end()) {
    		if (ptr_type != it->second) {
    			delete ptr_type;
    			return it->second;
    		} else return ptr_type;
        } else {
            this->ptr_types[ptr_type->base] = ptr_type;
            return ptr_type;
        }
    }

    bool func_type_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
        if (func_type1->parameter_decls.size() != func_type2->parameter_decls.size()) return false;
        if (func_type1->return_type != func_type2->return_type) return false;
        for (int i = 0; i < func_type1->parameter_decls.size(); i++) {
            auto decl_1 = func_type1->parameter_decls[i];
            auto decl_2 = func_type2->parameter_decls[i];
            // since we've already "uniqued" the parameter types, we can
            // check if they're the same using pointers
            if (decl_1->type != decl_2->type) return false;
        }
        return true;
    }

    Ast_Function_Type* get_unique_function_type (Ast_Function_Type* func_type) {
        for (auto _func_type : this->func_types) {
            if (func_type_are_equal(func_type, _func_type)) {
    			if (func_type != _func_type) {
    				delete func_type;
    				return _func_type;
    			} else return func_type;
            }
        }
        this->func_types.push_back(func_type);
        return func_type;
    }
};
