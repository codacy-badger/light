#include "util/logger.hpp"

Log_Level Logger::current_level = LOG_LEVEL_VERBOSE;
std::mutex Logger::mutex;