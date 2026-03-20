#pragma once

#include <vector>
#include "gtest/gtest.h"
#include "ProcessClass.hpp"

class ProcessTest : public ::testing::Test {
protected:
	ProcessClass process;
	std::vector<ProcessClass> processes;
	void SetUp() override;
	void TearDown() override;
};
