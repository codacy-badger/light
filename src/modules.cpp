#include "modules.hpp"

#include "compiler.hpp"
#include "platform.hpp"

#include "pipeline/pipes/external_modules.hpp"
#include "pipeline/pipes/symbol_resolution.hpp"
#include "pipeline/pipes/constant_propagation.hpp"
#include "pipeline/pipes/constant_folding.hpp"
#include "pipeline/pipes/cast_strings.hpp"
#include "pipeline/pipes/cast_arrays.hpp"
#include "pipeline/pipes/cast_anys.hpp"
#include "pipeline/pipes/type_checking.hpp"
#include "pipeline/pipes/foreign_function.hpp"
#include "pipeline/pipes/static_if.hpp"
#include "pipeline/pipes/call_arguments.hpp"

#include "pipeline/pipes/register_allocator.hpp"
#include "pipeline/pipes/bytecode_generator.hpp"
#include "pipeline/pipes/run_directive.hpp"

Modules::Modules (Compiler* compiler) {
    this->pipeline
        ->pipe(new External_Modules())

        ->pipe(new Foreign_Function())
        ->pipe(new Symbol_Resolution())
        ->pipe(new Constant_Propagation())
        ->pipe(new Static_If())
        ->pipe(new Cast_Strings())
        ->pipe(new Type_Checking())
        ->pipe(new Cast_Arrays())
        ->pipe(new Cast_Anys())
        ->pipe(new Constant_Folding())
        ->pipe(new Call_Arguments())

        ->pipe(new Register_Allocator(INTERP_REGISTER_COUNT))
        ->pipe(new Bytecode_Generator())
        ->pipe(new Run_Directive(compiler->interp));

    Events::add_observer(CE_MODULE_PARSED, &Modules::handle_module_parsed, this);
}

void Modules::handle_module_parsed (void* data) {
    auto module = reinterpret_cast<Module*>(data);

    this->pipeline->process(module->global_scope);
}

Module* Modules::get_module (const char* absolute_path) {
    if (!this->is_module_cached(absolute_path)) {
        return this->load_module(absolute_path);
    } else return this->cache[absolute_path];
}

Module* Modules::load_module (const char* absolute_path) {
    auto scanner = new Scanner(absolute_path);
    this->parser->tokens = new vector<Token*>();
    this->lexer->source_to_tokens(scanner, this->parser->tokens);
    delete scanner;

    auto module = new Module();

    module->global_scope = this->parser->build_ast();
    this->pipeline->process(module->global_scope);

    this->cache[absolute_path] = module;
    return module;
}

bool Modules::is_module_cached (const char* absolute_path) {
    auto it = this->cache.find(absolute_path);
    return it != this->cache.end();
}

void Modules::print_metrics (double userInterval) {
	printf("\n");
    this->lexer->print_metrics(userInterval);
    this->parser->print_metrics(userInterval);
    this->pipeline->print_metrics(userInterval);
}
