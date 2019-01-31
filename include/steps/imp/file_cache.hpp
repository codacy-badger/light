#pragma once

#include "steps/simple_pipe.hpp"

#include "utils/string_map.hpp"
#include "ast/nodes.hpp"

#include "code_source.hpp"

struct File_Cache : Simple_Pipe {
    String_Map<Ast_Scope*> cache;
    std::vector<const char*> in_progress;

    File_Cache() : Simple_Pipe("File Cache") { /* empty */ }

    void handle (void* in) {
        auto source = reinterpret_cast<Code_Source*>(in);

        if (!this->cache.contains(source->absolute_path)) {
            if (!this->is_in_progress(source->absolute_path)) {
                this->add_in_progress(source->absolute_path);
                this->pipe_out(in);
            }
        }
    }

    void on_module_ready (void* data) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(data);

        this->remove_in_progress(global_scope->get_absolute_path());
    }

    std::vector<const char*>::iterator find_in_progress (const char* absolute_path) {
        auto it = this->in_progress.begin();
        while (it != this->in_progress.end()) {
            if (strcmp(*it, absolute_path) == 0) {
                return it;
            } else it++;
        }
        return it;
    }

    bool is_in_progress (const char* absolute_path) {
        return this->find_in_progress(absolute_path) != this->in_progress.end();
    }

    void add_in_progress (const char* absolute_path) {
        this->in_progress.push_back(absolute_path);
    }

    void remove_in_progress (const char* absolute_path) {
        auto it = this->find_in_progress(absolute_path);
        if (it != this->in_progress.end()) {
            this->in_progress.erase(it);
        }
    }
};
