#pragma once

struct ASTBlock : ASTStatement {
	std::string name;
	std::vector<ASTStatement*> list;

	ASTBlock* parent = nullptr;
	map<std::string, ASTExpression*> symbols;

	ASTBlock (std::string name, ASTBlock* parent = nullptr) {
		this->parent = parent;
		this->name = name;
	}

	void add (string name, ASTExpression* val) {
		auto it = this->symbols.find(name);
		if (it == this->symbols.end()) {
			this->symbols[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
			exit(1);
		}
	}

	ASTExpression* get (string name) {
		auto it = this->symbols.find(name);
		if (it != this->symbols.end()) {
			return this->symbols[name];
		} else {
			if (this->parent)
				return this->parent->get(name);
			else return nullptr;
		}
	}

	template <typename T>
	T* get (string name) {
		if (auto casted = dynamic_cast<T*>(this->get(name)))
			return casted;
		else return nullptr;
	}
};
