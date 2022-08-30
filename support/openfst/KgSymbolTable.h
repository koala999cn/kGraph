#pragma once
#include "../../common/bimap.h"


class KgSymbolTable
{
public:
	KgSymbolTable() : maxKey_(0) {}
	KgSymbolTable(size_t tableSize) : maxKey_(0), table_(tableSize) {}


	void reserve(size_t mapSize) { table_.reserve(mapSize); }

	void clear() { table_.clear(); }

	auto size() const { return table_.size(); }

	void addSymbol(const std::string& symbol, std::int64_t key) {
		table_.insert(symbol, key);
		if (key >= maxKey_)
			maxKey_ = key + 1;
	}

	std::int64_t addSymbol(const std::string& symbol) {
		addSymbol(symbol, maxKey_);
		return maxKey_++;
	}

	// Removes the symbol with the given key.
	void eraseSymbol(std::int64_t key) {
		table_.eraseByY(key);
	}

	bool hasSymbol(const std::string& symbol) const {
		return table_.hasX(symbol);
	}

	bool hasKey(std::int64_t key) const {
		return table_.hasY(key);
	}

	// return the string associated with the key
	std::string getSymbol(std::int64_t key) const {
		return table_.y2x(key);
	}

	// return the key associated with the symbol
	std::int64_t getKey(const std::string& symbol) {
		return table_.x2y(symbol);
	}

	auto begin() const { return table_.xyBegin(); }
	auto end() const { return table_.xyEnd(); }


private:
	bimap<std::string, std::int64_t> table_;
	std::int64_t maxKey_;
};