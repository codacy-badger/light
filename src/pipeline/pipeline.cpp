#include "pipeline/pipeline.hpp"

#include "pipeline/pipes/compiler_directives.hpp"
#include "pipeline/pipes/symbol_resolution.hpp"
#include "pipeline/pipes/constant_folding.hpp"
#include "pipeline/pipes/cast_arrays.hpp"
#include "pipeline/pipes/type_checking.hpp"
#include "pipeline/pipes/foreign_function.hpp"

#include "pipeline/pipes/register_allocator.hpp"
#include "pipeline/pipes/bytecode_generator.hpp"
#include "pipeline/pipes/bytecode_runner.hpp"

#define DECL_TYPE(scope, type) scope->statements.push_back(ast_make_declaration(type->name, type));

Pipeline::Pipeline() {
    this->pipes.push_back(new Compiler_Directives());
    this->pipes.push_back(new Foreign_Function());
    this->pipes.push_back(new Symbol_Resolution());
    this->pipes.push_back(new Type_Checking());
    this->pipes.push_back(new Cast_Arrays());
    this->pipes.push_back(new Constant_Folding());

    this->pipes.push_back(new Register_Allocator());
    this->pipes.push_back(new Bytecode_Generator());

    this->pipes.push_back(new Bytecode_Runner());
}

void Pipeline::run(const char* filepath) {
    printf("\n%s\n", filepath);

    this->global_scope = Ast_Factory::create_node<Ast_Scope>();
    DECL_TYPE(this->global_scope, Types::type_type);
    DECL_TYPE(this->global_scope, Types::type_void);
    DECL_TYPE(this->global_scope, Types::type_bool);
    DECL_TYPE(this->global_scope, Types::type_s8);
    DECL_TYPE(this->global_scope, Types::type_s16);
    DECL_TYPE(this->global_scope, Types::type_s32);
    DECL_TYPE(this->global_scope, Types::type_s64);
    DECL_TYPE(this->global_scope, Types::type_u8);
    DECL_TYPE(this->global_scope, Types::type_u16);
    DECL_TYPE(this->global_scope, Types::type_u32);
    DECL_TYPE(this->global_scope, Types::type_u64);
    DECL_TYPE(this->global_scope, Types::type_f32);
    DECL_TYPE(this->global_scope, Types::type_f64);
    this->imported_files.clear();

    auto start = os_get_user_time();

    this->handle_file(filepath);

    print_compiler_metrics(os_time_user_stop(start));

    delete this->global_scope;
    this->global_scope = NULL;
}

void Pipeline::handle_file(const char* filepath) {
    char last_path[MAX_PATH_LENGTH];
    os_get_current_directory(last_path);
    os_set_current_directory_path(filepath);

    this->parser->setup(filepath, this->global_scope);
	this->imported_files.push_back(filepath);

	auto start = os_get_user_time();
    auto stm = this->parser->statement();
	this->parser->parsing_time += os_time_user_stop(start);

    while (stm != NULL) {
		this->parser->push(stm);

        this->handle_stm(stm);

        start = os_get_user_time();
        stm = this->parser->statement();
    	this->parser->parsing_time += os_time_user_stop(start);
    }

    for (auto pipe : this->pipes) {
        pipe->on_finish();
    }

    this->parser->teardown();

    os_set_current_directory(last_path);
}

void Pipeline::handle_stm(Ast_Statement* stm, int from_index) {
    if (stm->stm_type == AST_STATEMENT_IMPORT) {
        auto import = static_cast<Ast_Import*>(stm);
        this->handle_import(import);
        return;
    }

    for (int i = from_index; i < this->pipes.size(); i++) {
        auto pipe = this->pipes[i];

        pipe->on_statement(&stm);

        while (pipe->pending_stms.size()) {
            auto _stm = pipe->pending_stms.front();
            pipe->pending_stms.pop_front();

            this->handle_stm(_stm, i + 1);
        }

        if (pipe->stop_processing) {
            pipe->stop_processing = false;
            break;
        }
    }
}

void Pipeline::handle_import(Ast_Import* import) {
    auto literal = static_cast<Ast_Literal*>(import->target);

    import->absolute_path = (char*) malloc(MAX_PATH_LENGTH);
    os_get_absolute_path(literal->string_value, import->absolute_path);

    for (auto imported_file : this->imported_files) {
        if (strcmp(imported_file, import->absolute_path) == 0) {
            free(import->absolute_path);
            import->absolute_path = NULL;
            return;
        }
    }

    this->handle_file(import->absolute_path);
}

void Pipeline::print_compiler_metrics (double total_time) {
	printf("\n");

    double percent = (this->parser->parsing_time * 100.0) / total_time;
    printf("  - %-25s%8.6fs (%5.2f%%)\n", "Parser & Lexer",
        this->parser->parsing_time, percent);
    this->parser->print_metrics();

    for (auto pipe : this->pipes) {
        if (pipe->pipe_name) {
			percent = (pipe->total_time * 100.0) / total_time;
			printf("  - %-25s%8.6fs (%5.2f%%)\n", pipe->pipe_name, pipe->total_time, percent);
			pipe->print_pipe_metrics();
		}
    }
	printf("\n  Total Time                 %8.6fs\n", total_time);
}
