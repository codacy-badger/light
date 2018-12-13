#include "modules.hpp"

#include "pipeline/pipes/external_modules.hpp"
#include "pipeline/pipes/symbol_resolution.hpp"
#include "pipeline/pipes/constant_folding.hpp"
#include "pipeline/pipes/cast_arrays.hpp"
#include "pipeline/pipes/type_checking.hpp"
#include "pipeline/pipes/foreign_function.hpp"
#include "pipeline/pipes/static_if.hpp"

#include "pipeline/pipes/register_allocator.hpp"
#include "pipeline/pipes/bytecode_generator.hpp"
#include "pipeline/pipes/bytecode_runner.hpp"

#define DECL_TYPE(scope, type) scope->statements.push_back(ast_make_declaration(type->name, type));

Modules::Modules () {
    DECL_TYPE(this->internal_scope, Types::type_type);
    DECL_TYPE(this->internal_scope, Types::type_void);
    DECL_TYPE(this->internal_scope, Types::type_bool);
    DECL_TYPE(this->internal_scope, Types::type_s8);
    DECL_TYPE(this->internal_scope, Types::type_s16);
    DECL_TYPE(this->internal_scope, Types::type_s32);
    DECL_TYPE(this->internal_scope, Types::type_s64);
    DECL_TYPE(this->internal_scope, Types::type_u8);
    DECL_TYPE(this->internal_scope, Types::type_u16);
    DECL_TYPE(this->internal_scope, Types::type_u32);
    DECL_TYPE(this->internal_scope, Types::type_u64);
    DECL_TYPE(this->internal_scope, Types::type_f32);
    DECL_TYPE(this->internal_scope, Types::type_f64);

    auto windown_constant = ast_make_declaration("OS_WINDOWS", ast_make_literal(true));
    windown_constant->type = Types::type_bool;
    auto linux_constant = ast_make_declaration("OS_LINUX", ast_make_literal(false));
    linux_constant->type = Types::type_bool;

    this->internal_scope->statements.push_back(windown_constant);
    this->internal_scope->statements.push_back(linux_constant);

    this->pipeline
        ->pipe(new External_Modules())

        ->pipe(new Foreign_Function())
        ->pipe(new Symbol_Resolution())
        ->pipe(new Type_Checking())
        ->pipe(new Cast_Arrays())
        ->pipe(new Constant_Folding())
        ->pipe(new Static_If())

        ->pipe(new Register_Allocator())
        ->pipe(new Bytecode_Generator())
        ->pipe(new Bytecode_Runner());
}

Ast_Scope* Modules::get_module (char* absolute_path) {
    if (!this->is_module_cached(absolute_path)) {
        return this->load_module(absolute_path);
    } else return this->cache[absolute_path];
}

bool Modules::is_module_cached (char* absolute_path) {
    auto it = this->cache.find(absolute_path);
    return it != this->cache.end();
}

Ast_Scope* Modules::load_module (char* absolute_path) {
    char last_path[MAX_PATH_LENGTH];
    os_get_current_directory(last_path);
    os_set_current_directory_path(absolute_path);

    auto file_scope = this->parser->run(absolute_path, this->internal_scope);

    this->pipeline->process(file_scope);
    this->cache[absolute_path] = file_scope;

    os_set_current_directory(last_path);

    return file_scope;
}

bool Modules::free_module (char* absolute_path) {
    if (this->is_module_cached(absolute_path)) {
        this->cache.erase(absolute_path);
        return true;
    }
    return false;
}
