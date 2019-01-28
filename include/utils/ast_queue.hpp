#pragma once

#include "async_queue.hpp"
#include "ast/nodes.hpp"

typedef Async_Queue<Ast_Statement*> Ast_Queue;
