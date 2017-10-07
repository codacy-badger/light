#pragma once

struct ASTScope : ASTStatement {
	std::string name;
	std::vector<ASTStatement*> list;

	ASTScope* parent = nullptr;
	map<string, ASTExpression*> variables;
	map<string, vector<void*>> unresolved;

	ASTScope (std::string name, ASTScope* parent = nullptr) {
		this->parent = parent;
		this->name = name;
	}

	void add (string name, ASTExpression* val) {
		this->resolve(name, val);
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			this->variables[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
			exit(1);
		}
	}

	ASTExpression* get (string name) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			if (this->parent == nullptr)  return nullptr;
			else return this->parent->get(name);
		} else return this->variables[name];
	}

	void addUnresolved (string name, void* addr) {
		auto it = unresolved.find(name);
		if (it == unresolved.end()) {
			vector<void*> toEdit { addr };
			unresolved[name] = toEdit;
		} else unresolved[name].push_back(addr);
	}

	bool resolve (string name, ASTExpression* value) {
		auto it = this->unresolved.find(name);
		if (it != this->unresolved.end()) {
			for (auto addrs : it->second)
				memcpy(addrs, &value, sizeof(ASTExpression*));
			this->unresolved.erase(it);
			return true;
		} else return false;
	}
};
