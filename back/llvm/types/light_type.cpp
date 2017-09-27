#pragma once

#include <string>
#include <vector>
#include <map>

#include "parser/parser.cpp"

class LiType {
public:
	std::string name;
	llvm::Type* llvmType = nullptr;

	LiType (std::string name) {
		this->name = name;
	}
};
