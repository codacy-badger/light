#include "ast/nodes.hpp"

void Ast_Scope::get_all_declarations (String_Map<std::vector<Ast_Declaration*>>* decl_map) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            for (size_t i = 0; i < decl->names.size; i++) {
                auto decl_name = decl->names[i];
                if (decl_name) {
                    (*decl_map)[decl_name].push_back(decl);
                }
            }
        }
    }
}