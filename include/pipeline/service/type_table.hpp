#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/types.hpp"

#include <vector>

#define MAX_TYPE_NAME_LENGTH 1024

struct Type_Table {
    Build_Context* context = NULL;

    std::vector<Ast_Type*> global_type_table;

    char type_name_buffer[MAX_TYPE_NAME_LENGTH];

    void init (Build_Context* c) { this->context = c; }

    void unique (Ast_Type** type_ptr) {
        auto type = (*type_ptr);

        if (type->guid > 0) return;

        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                this->unique(reinterpret_cast<Ast_Struct_Type**>(type_ptr));
                assert(type->guid > 0);
                break;
            }
            case AST_TYPEDEF_POINTER: {
                this->unique(reinterpret_cast<Ast_Pointer_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->unique(reinterpret_cast<Ast_Array_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                this->unique(reinterpret_cast<Ast_Function_Type**>(type_ptr));
                break;
            }
            default: assert(false);
        }
    }

    void unique (Ast_Struct_Type** struct_ptr) {
        auto _struct = (*struct_ptr);

        if (_struct->guid > 0) return;

        _struct->guid = this->global_type_table.size() + 1;
        this->global_type_table.push_back(_struct);
        this->compute_type_name_if_needed(_struct);
    }

    void unique (Ast_Pointer_Type** ptr_type_ptr) {
        auto ptr_type = (*ptr_type_ptr);

        if (ptr_type->guid > 0) return;

        assert(ptr_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&ptr_type->typed_base);

        for (auto type : this->global_type_table) {
            if (type->typedef_type == AST_TYPEDEF_POINTER) {
                auto uniqued_ptr_type = static_cast<Ast_Pointer_Type*>(type);
                assert(uniqued_ptr_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_ptr_type->typed_base->guid == ptr_type->typed_base->guid) {
                    (*ptr_type_ptr) = uniqued_ptr_type;
                    return;
                }
            }
        }

        ptr_type->guid = this->global_type_table.size() + 1;
        this->global_type_table.push_back(ptr_type);
        this->compute_type_name_if_needed(ptr_type);
    }

    void unique (Ast_Array_Type** array_type_ptr) {
        auto array_type = (*array_type_ptr);

        if (array_type->guid > 0) return;

        assert(array_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&array_type->typed_base);

        for (auto type : this->global_type_table) {
            if (type->typedef_type == AST_TYPEDEF_ARRAY) {
                auto uniqued_array_type = static_cast<Ast_Array_Type*>(type);
                assert(uniqued_array_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_array_type->typed_base->guid == array_type->typed_base->guid) {
                    (*array_type_ptr) = uniqued_array_type;
                    return;
                }
            }
        }

        array_type->guid = this->global_type_table.size() + 1;
        this->global_type_table.push_back(array_type);
        this->compute_type_name_if_needed(array_type);
    }

    void unique (Ast_Function_Type** func_type_ptr) {
        auto func_type = (*func_type_ptr);

        if (func_type->guid > 0) return;

        assert(func_type->ret_type->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&func_type->typed_ret_type);
        for (auto& arg_decl : func_type->arg_decls) {
            assert(arg_decl->type->exp_type == AST_EXPRESSION_TYPE);
            this->unique(&arg_decl->typed_type);
        }

        for (auto type : this->global_type_table) {
            if (type->typedef_type == AST_TYPEDEF_FUNCTION) {
                auto uniqued_function_type = static_cast<Ast_Function_Type*>(type);

                if (this->types_are_equal(uniqued_function_type, func_type)) {
                    (*func_type_ptr) = uniqued_function_type;
                    return;
                }
            }
        }

        func_type->guid = this->global_type_table.size() + 1;
        this->global_type_table.push_back(func_type);
        this->compute_type_name_if_needed(func_type);
    }

    bool types_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
        if (func_type1 == func_type2) return true;

        if (func_type1->arg_decls.size() != func_type2->arg_decls.size()) return false;

        if (func_type1->typed_ret_type != func_type2->typed_ret_type) return false;

        for (size_t i = 0; i < func_type1->arg_decls.size(); i++) {
            auto attr1 = func_type1->arg_decls[i];
            auto attr2 = func_type2->arg_decls[i];

            if (attr1->typed_type != attr2->typed_type) return false;
        }

        return true;
    }

    void compute_type_name_if_needed (Ast_Type* type) {
        if (type->name) return;

		switch (type->typedef_type) {
	        case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type);
				if (_struct->is_slice) {
					auto slice = static_cast<Ast_Slice_Type*>(type);

                    auto base = slice->get_base();
                    assert(base->exp_type == AST_EXPRESSION_TYPE);
                    auto base_type = static_cast<Ast_Type*>(base);
                    this->compute_type_name_if_needed(base_type);

                    memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                    sprintf_s(this->type_name_buffer, strlen(base_type->name) + 3,
                        "[]%s", base_type->name);

                    slice->name = _strdup(this->type_name_buffer);
				}
				break;
			}
	        case AST_TYPEDEF_POINTER: {
	            auto _ptr = static_cast<Ast_Pointer_Type*>(type);

                assert(_ptr->base->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(_ptr->typed_base);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(_ptr->typed_base->name) + 2,
                    "*%s", _ptr->typed_base->name);

                _ptr->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_ARRAY: {
	            auto _arr = static_cast<Ast_Array_Type*>(type);
                assert(_arr->length_uint > 0);

                assert(_arr->base->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(_arr->typed_base);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(_arr->typed_base->name) + 2,
                    "[%lld]%s", _arr->length_uint, _arr->typed_base->name);

                _arr->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_FUNCTION: {
	            auto _func = static_cast<Ast_Function_Type*>(type);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, "fn (");
                if (_func->arg_decls.size() > 0) {
                    auto arg_decl = _func->arg_decls[0];
                    assert(arg_decl->type->exp_type == AST_EXPRESSION_TYPE);
                    this->compute_type_name_if_needed(arg_decl->typed_type);
                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, arg_decl->typed_type->name);
                    for (size_t i = 1; i < _func->arg_decls.size(); i++) {
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ", ");

                        arg_decl = _func->arg_decls[i];
                        assert(arg_decl->type->exp_type == AST_EXPRESSION_TYPE);
                        this->compute_type_name_if_needed(arg_decl->typed_type);
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, arg_decl->typed_type->name);
                    }
                }
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ") -> ");

                assert(_func->ret_type->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(_func->typed_ret_type);
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, _func->typed_ret_type->name);

                _func->name = _strdup(this->type_name_buffer);

                break;
	        }
	        default: assert(false);
	    }
    }
};
