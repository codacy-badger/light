#pragma once

#include "async_queue.hpp"
#include "compiler_events.hpp"

typedef Async_Queue<Compiler_Event*> Event_Queue;
