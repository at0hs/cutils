#pragma once

#include "gtest/gtest.h"
#include <Bitset/Bitset.h>
#include "BitsetCLinkage.h"
#include <bitset>
#include <vector>

class BitsetTest : public ::testing::Test {
protected:
	static constexpr std::uint64_t numBits = 128;
	Bitset_t bitset;
	std::bitset<numBits> stlBitset;
	void SetUp() override;
	void TearDown() override;
	std::vector<std::uint64_t> Get();
};
