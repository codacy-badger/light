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

struct Cast_Instance {
    Ast_Function* function = NULL;
    bool is_implicid = false;
};

struct Types {
    map<const char*, Ast_Struct_Type*, cmp_str_types> struct_types;
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Expression*, Ast_Array_Type*> arr_types;
    vector<Ast_Function_Type*> func_types;

    map<Ast_Type_Definition*, map<Ast_Type_Definition*, Cast_Instance*>> casts;

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
            case AST_TYPEDEF_ARRAY: {
                auto _arr = static_cast<Ast_Array_Type*>(type_def);
                return this->get_unique_array_type(_arr);
            }
            case AST_TYPEDEF_FUNCTION: {
                auto _func = static_cast<Ast_Function_Type*>(type_def);
                return this->get_unique_function_type(_func);
            }
            default: return NULL;
        }
    }

	Ast_Struct_Type* get_struct_type (const char* name) {
		auto it = this->struct_types.find(name);
        if (it != this->struct_types.end()) {
    		return it->second;
        } else return NULL;
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

	Ast_Pointer_Type* get_pointer_type (Ast_Type_Definition* base_type) {
		auto it = this->ptr_types.find(base_type);
        if (it != this->ptr_types.end()) {
    		return it->second;
        } else return NULL;
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

    Ast_Array_Type* get_unique_array_type (Ast_Array_Type* arr_type) {
        auto it = this->arr_types.find(arr_type->base);
        if (it != this->arr_types.end()) {
            if (arr_type != it->second && arr_type->kind == it->second->kind) {
                switch (arr_type->kind) {
                    case AST_ARRAY_KIND_SLICE: {
                        delete arr_type;
                        return it->second;
                    }
                    case AST_ARRAY_KIND_STATIC: {
                        if (it->second->length() == arr_type->length()) {
                            delete arr_type;
                			return it->second;
                		}
                        break;
                    }
                }
            }
        }
        this->arr_types[arr_type->base] = arr_type;
        return arr_type;
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

    void add_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to, Ast_Function* func, bool is_implicid = false) {
        auto cast_instance = new Cast_Instance();
        cast_instance->function = func;
        cast_instance->is_implicid = is_implicid;
        this->casts[type_from][type_to] = cast_instance;
    }

    Cast_Instance* get_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to) {
        auto it = this->casts.find(type_from);
        if (it != this->casts.end()) {
            auto it2 = it->second.find(type_to);
            if (it2 != it->second.end()) {
                return it2->second;
            } else return NULL;
        } else return NULL;
    }

    bool is_implicid_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to) {
		if (type_from->typedef_type == AST_TYPEDEF_POINTER && type_to->typedef_type == AST_TYPEDEF_POINTER) {
			auto ptr_type_from = static_cast<Ast_Pointer_Type*>(type_from);
			auto ptr_type_to = static_cast<Ast_Pointer_Type*>(type_to);
			return ptr_type_from->base == ptr_type_to->base;
		} else if (type_from->is_signed && type_to->is_signed) {
			return type_to->byte_size >= type_from->byte_size;
		} else if (!type_from->is_signed && type_to->is_signed) {
			return type_to->byte_size > type_from->byte_size;
		} else if (type_from->is_signed && !type_to->is_signed) {
			return false;
		} else {
			return type_to->byte_size >= type_from->byte_size;
		}
    }

    void compute_type_name_if_needed (Ast_Type_Definition* type_def) {
        switch (type_def->typedef_type) {
            case AST_TYPEDEF_STRUCT: return;
            case AST_TYPEDEF_POINTER: {
                auto _ptr = static_cast<Ast_Pointer_Type*>(type_def);
                if (_ptr->name == NULL) {
            		auto base_type_def = static_cast<Ast_Type_Definition*>(_ptr->base);
            		auto base_name_length = strlen(base_type_def->name);
            		_ptr->name = (char*) malloc(base_name_length + 2);
            		_ptr->name[0] = '*';
            		memcpy(_ptr->name + 1, base_type_def->name, base_name_length);
            		_ptr->name[base_name_length + 1] = '\0';
            	}
                return;
            }
            case AST_TYPEDEF_ARRAY: {
                auto _arr = static_cast<Ast_Array_Type*>(type_def);
                if (_arr->name == NULL) {
            		auto base_type_def = static_cast<Ast_Type_Definition*>(_arr->base);
            		auto base_name_length = strlen(base_type_def->name);
                    switch (_arr->kind) {
                        case AST_ARRAY_KIND_STATIC: {
                    		_arr->name = (char*) malloc(base_name_length + 23);
                            sprintf_s(_arr->name, base_name_length + 23, "[%lld]%s", _arr->length(), base_type_def->name);
                            break;
                        }
                        case AST_ARRAY_KIND_SLICE: {
                    		_arr->name = (char*) malloc(base_name_length + 3);
                            sprintf_s(_arr->name, base_name_length + 3, "[]%s", base_type_def->name);
                            break;
                        }
                    }
            	}
                return;
            }
            case AST_TYPEDEF_FUNCTION: {
                auto _func = static_cast<Ast_Function_Type*>(type_def);
                if (_func->name == NULL) {
            		auto par_decls = _func->parameter_decls;

					Ast_Type_Definition* par_type_def;
            		size_t name_size = strlen("fn (");
            		if (par_decls.size() > 0) {
            			par_type_def = static_cast<Ast_Type_Definition*>(par_decls[0]->type);
            			name_size += strlen(par_type_def->name);
            			for (int i = 1; i < par_decls.size(); i++) {
            				name_size += strlen(", ");
            				par_type_def = static_cast<Ast_Type_Definition*>(par_decls[i]->type);
            				name_size += strlen(par_type_def->name);
            			}
            		}
            		name_size += strlen(") -> ");
            		par_type_def = static_cast<Ast_Type_Definition*>(_func->return_type);
            		name_size += strlen(par_type_def->name);
            		_func->name = (char*) malloc(name_size + 1);

            		size_t offset = 0;
            		memcpy(_func->name, "fn (", 4);
            		offset += 4;

					size_t par_type_name_length;
            		if (par_decls.size() > 0) {
            			par_type_def = static_cast<Ast_Type_Definition*>(par_decls[0]->type);
						par_type_name_length = strlen(par_type_def->name);
            			memcpy(_func->name + offset, par_type_def->name, par_type_name_length);
            			offset += par_type_name_length;
            			for (int i = 1; i < par_decls.size(); i++) {
            				memcpy(_func->name + offset, ", ", 2);
            				offset += 2;
                            par_type_def = static_cast<Ast_Type_Definition*>(par_decls[i]->type);
							par_type_name_length = strlen(par_type_def->name);
            				memcpy(_func->name + offset, par_type_def->name, par_type_name_length);
            				offset += par_type_name_length;
            			}
            		}

            		memcpy(_func->name + offset, ") -> ", 5);
            		offset += 5;
            		par_type_def = static_cast<Ast_Type_Definition*>(_func->return_type);
					par_type_name_length = strlen(par_type_def->name);
            		memcpy(_func->name + offset, par_type_def->name, par_type_name_length);
            		offset += par_type_name_length;
            		_func->name[offset] = '\0';
            	}
                return;
            }
            default: return;
        }
    }
};
