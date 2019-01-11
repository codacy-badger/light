#pragma once

#include <map>

struct cmp_str {
   bool operator()(char const *a, char const *b) const {
      return std::strcmp(a, b) < 0;
   }
};

template<typename T>
struct String_Map : std::map<const char*, T, cmp_str> { /* empty */ };
