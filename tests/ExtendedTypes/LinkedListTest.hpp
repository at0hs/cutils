#pragma once

#include "gtest/gtest.h"
#include "LinkedListWrapper.hpp"

class LinkedListTest : public ::testing::Test {
protected:
	LinkedListWrapper<int> list{};
};
