#pragma once

#include "gtest/gtest.h"
#include "Stream/MemoryStream.h"

class MemoryStreamTest : public ::testing::Test {
protected:
	MemoryStreamTest();
	virtual ~MemoryStreamTest();
	virtual void SetUp() override;
	virtual void TearDown() override;

	MemoryStream memoryStream;
};
