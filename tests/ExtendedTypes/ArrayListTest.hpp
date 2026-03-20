#pragma once

#include "gtest/gtest.h"
#include "ArrayListWrapper.hpp"

class ArrayListTest : public ::testing::Test {
protected:
	ArrayListWrapper<int> list{};
};
