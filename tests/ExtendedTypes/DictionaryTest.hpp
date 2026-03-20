#pragma once

extern "C" {
#include "ExtendedTypes/Dictionary.h"
}
#include <string>
#include <cstring>
#include "gtest/gtest.h"

class DictionaryTest : public ::testing::Test {
protected:
	size_t capacity = 3;
	size_t maxKeySize = 8;
	size_t maxObjectSize = 4;
	std::string keys[3] = { "apple", "banana", "orange" };
	int objects[3] = { 120, 200, 150 };
	Dictionary_t dict;
	virtual void SetUp();
	virtual void TearDown();
public:
	void AddAll();
};
