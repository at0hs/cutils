// NOLINTBEGIN
#include "BitsetTest.hpp"

void BitsetTest::SetUp() {
	Bitset_Init(&bitset, 128);
}

void BitsetTest::TearDown() {
	Bitset_Destroy(&bitset);
}

std::vector<std::uint64_t> BitsetTest::Get() {
	return std::vector<std::uint64_t>(bitset.words, bitset.words + bitset.capacity);
}

TEST_F(BitsetTest, Initialize) {
	Bitset_t bs;
	Bitset_Init(&bs, 8);
	EXPECT_EQ(1, bs.capacity);
	Bitset_Destroy(&bs);

	Bitset_Init(&bs, 100);
	EXPECT_EQ(2, bs.capacity);
	Bitset_Destroy(&bs);
}

TEST_F(BitsetTest, Or) {
	std::uint64_t input1 = 1;
	Bitset_Or(&bitset, input1);
	auto output = Get();
	EXPECT_EQ(input1, output[0]);
	EXPECT_EQ(0, output[1]);

	std::uint64_t input2 = 0xf1;
	Bitset_Or(&bitset, input2);
	output = Get();
	EXPECT_EQ(input1 | input2, output[0]);
	EXPECT_EQ(0, output[1]);
}

TEST_F(BitsetTest, And) {
	std::uint64_t input{ 0x0f };
	Bitset_And(&bitset, input);
	std::vector<std::uint64_t> output1 = Get();
	EXPECT_EQ(0, output1[0]);
	EXPECT_EQ(0, output1[1]);

	Bitset_Or(&bitset, 0xff);
	Bitset_And(&bitset, input);
	std::vector<std::uint64_t> output2 = Get();
	EXPECT_EQ(input, output2[0]);
	EXPECT_EQ(0, output2[1]);
}

TEST_F(BitsetTest, ExclusiveOr) {
	std::uint64_t input{ 0x0f };
	Bitset_Or(&bitset, input);
	Bitset_ExclusiveOr(&bitset, input);
	std::vector<std::uint64_t> output = Get();
	EXPECT_EQ(0, output[0]);
	EXPECT_EQ(0, output[1]);
}

TEST_F(BitsetTest, Clear) {
	Bitset_Or(&bitset, 0xff);
	Bitset_Clear(&bitset);
	std::vector<std::uint64_t> output = Get();
	EXPECT_EQ(0, output[0]);
	EXPECT_EQ(0, output[1]);
}

TEST_F(BitsetTest, Set) {
	Bitset_Set(&bitset, 0);
	Bitset_Set(&bitset, 64);
	std::vector<std::uint64_t> output1 = Get();
	EXPECT_EQ(1, output1[0]);
	EXPECT_EQ(1, output1[1]);

	Bitset_Set(&bitset, numBits + 1);
	std::vector<std::uint64_t> output2 = Get();
	EXPECT_EQ(1, output2[0]);
	EXPECT_EQ(1, output2[1]);
}

TEST_F(BitsetTest, Reset) {
	Bitset_Set(&bitset, 0);
	Bitset_Set(&bitset, 64);

	Bitset_Reset(&bitset, numBits + 1);
	std::vector<std::uint64_t> output1 = Get();
	EXPECT_EQ(1, output1[0]);
	EXPECT_EQ(1, output1[1]);

	Bitset_Reset(&bitset, 0);
	Bitset_Reset(&bitset, 64);
	std::vector<std::uint64_t> output2 = Get();
	EXPECT_EQ(0, output2[0]);
	EXPECT_EQ(0, output2[1]);
}

TEST_F(BitsetTest, Flip) {
	uint64_t input{ 0x0f };
	Bitset_Or(&bitset, input);
	Bitset_Flip(&bitset);
	std::vector<std::uint64_t> output = Get();
	EXPECT_EQ(~input, output[0]);
	EXPECT_EQ(~0UL, output[1]);
}

TEST_F(BitsetTest, Count) {
	Bitset_Set(&bitset, 0);
	Bitset_Set(&bitset, 64);
	EXPECT_EQ(2, Bitset_Count(&bitset));
}

TEST_F(BitsetTest, Test) {
	Bitset_Set(&bitset, 0);
	Bitset_Set(&bitset, 64);
	EXPECT_EQ(true, Bitset_Test(&bitset, 0));
	EXPECT_EQ(false, Bitset_Test(&bitset, 1));
	EXPECT_EQ(true, Bitset_Test(&bitset, 64));
	EXPECT_EQ(false, Bitset_Test(&bitset, 65));

	EXPECT_EQ(false, Bitset_Test(&bitset, numBits + 1));
}

TEST_F(BitsetTest, Any) {
	EXPECT_EQ(false, Bitset_Any(&bitset));
	Bitset_Set(&bitset, 0);
	EXPECT_EQ(true, Bitset_Any(&bitset));
}

TEST_F(BitsetTest, All) {
	Bitset_Or(&bitset, 0xf);
	EXPECT_EQ(false, Bitset_All(&bitset));
	Bitset_Clear(&bitset);
	Bitset_Flip(&bitset);
	EXPECT_EQ(true, Bitset_All(&bitset));
}

TEST_F(BitsetTest, None) {
	EXPECT_EQ(true, Bitset_None(&bitset));
	Bitset_Set(&bitset, 0);
	EXPECT_EQ(false, Bitset_None(&bitset));
}

TEST_F(BitsetTest, LeftShift) {
	std::uint64_t input{ 0x0f };
	Bitset_Or(&bitset, input);

	Bitset_LeftShift(&bitset, 64);
	std::vector<std::uint64_t> output1 = Get();
	EXPECT_EQ(0, output1[0]);
	EXPECT_EQ(input, output1[1]);

	Bitset_LeftShift(&bitset, 4);
	std::vector<std::uint64_t> output2 = Get();
	EXPECT_EQ(0, output2[0]);
	EXPECT_EQ(input << 4, output2[1]);

	Bitset_LeftShift(&bitset, 200);
	std::vector<std::uint64_t> output3 = Get();
	EXPECT_EQ(0, output3[0]);
	EXPECT_EQ(0, output3[1]);
}

TEST_F(BitsetTest, RightShift) {
	std::uint64_t input{ 0x0f };
	Bitset_Or(&bitset, input);
	Bitset_LeftShift(&bitset, 64);

	Bitset_RightShift(&bitset, 64);
	std::vector<std::uint64_t> output1 = Get();
	EXPECT_EQ(input, output1[0]);
	EXPECT_EQ(0, output1[1]);

	Bitset_LeftShift(&bitset, 64);
	Bitset_RightShift(&bitset, 2);
	std::vector<std::uint64_t> output2 = Get();
	EXPECT_EQ(input << 62, output2[0]);
	EXPECT_EQ(input >> 2, output2[1]);

	Bitset_RightShift(&bitset, 200);
	std::vector<std::uint64_t> output3 = Get();
	EXPECT_EQ(0, output3[0]);
	EXPECT_EQ(0, output3[1]);
}

TEST_F(BitsetTest, ToString) {
	Bitset_Set(&bitset, 0);
	Bitset_Set(&bitset, 64);
	stlBitset.set(0);
	stlBitset.set(64);
	EXPECT_STREQ(stlBitset.to_string().c_str(), Bitset_ToString(&bitset));
}

TEST_F(BitsetTest, SegmentationFault) {
	Bitset_Init(nullptr, 0);
	Bitset_Destroy(nullptr);

	Bitset_RightShift(nullptr, 1);

	Bitset_LeftShift(nullptr, 1);

	Bitset_ToString(nullptr);

	Bitset_Clear(nullptr);

	Bitset_And(nullptr, 0);

	Bitset_Or(nullptr, 0);

	Bitset_ExclusiveOr(nullptr, 0);

	Bitset_Set(nullptr, 0);

	Bitset_Reset(nullptr, 0);

	Bitset_Flip(nullptr);

	EXPECT_EQ(0, Bitset_Count(nullptr));

	EXPECT_EQ(false, Bitset_Test(nullptr, 0));

	EXPECT_EQ(false, Bitset_Any(nullptr));

	EXPECT_EQ(false, Bitset_All(nullptr));

	EXPECT_EQ(false, Bitset_None(nullptr));
}

TEST(BitsetRegressionTest, InlineFunctionsLinkFromC) {
	// C (-O0) から inline 関数を呼んでもリンクでき、正しく動作すること
	EXPECT_EQ(113u, BitsetCLinkage_Run());
}

TEST(BitsetRegressionTest, LeftShiftDropsBitsBeyondNumBits) {
	Bitset_t bs;
	Bitset_Init(&bs, 4);
	Bitset_Set(&bs, 3);
	Bitset_LeftShift(&bs, 1);
	EXPECT_STREQ("0000", Bitset_ToString(&bs));
	EXPECT_EQ(0u, Bitset_Count(&bs));
	EXPECT_FALSE(Bitset_Any(&bs));
	EXPECT_TRUE(Bitset_None(&bs));
	// 押し出されたビットが右シフトで復活しないこと
	Bitset_RightShift(&bs, 1);
	EXPECT_STREQ("0000", Bitset_ToString(&bs));
	Bitset_Destroy(&bs);
}

TEST(BitsetRegressionTest, LeftShiftAcrossWordsDropsBitsBeyondNumBits) {
	Bitset_t bs;
	Bitset_Init(&bs, 70);
	Bitset_Set(&bs, 69);
	Bitset_Set(&bs, 10);
	Bitset_LeftShift(&bs, 64);
	EXPECT_EQ(0u, Bitset_Count(&bs));  // bit10 -> 74 (範囲外), bit69 -> 133 (範囲外)
	Bitset_Destroy(&bs);
}

TEST(BitsetRegressionTest, WordOperationsDoNotSetBitsBeyondNumBits) {
	Bitset_t bs;
	Bitset_Init(&bs, 4);
	Bitset_Or(&bs, 0xFF);
	EXPECT_EQ(4u, Bitset_Count(&bs));
	EXPECT_TRUE(Bitset_All(&bs));
	EXPECT_STREQ("1111", Bitset_ToString(&bs));
	Bitset_ExclusiveOr(&bs, 0xF0);
	EXPECT_EQ(4u, Bitset_Count(&bs));
	Bitset_Destroy(&bs);
}

// NOLINTEND
