// NOLINTBEGIN
#include "CsvParserTest.hpp"
#include <string>
#include <vector>

void CsvParserTest::SetUp() {
	parser = CsvParser_Init(&props);
	ASSERT_NE(nullptr, parser);
}

void CsvParserTest::TearDown() {
	CsvParser_Destroy(parser);
}

void CsvParserTest::AssertContent(CsvContent_t *expected) {
	const CsvContent_t *actual = CsvParser_GetContent(parser);
	ASSERT_TRUE(CsvContent_Equals(expected, actual));
}

TEST_F(CsvParserTest, Scenario1) {
	std::string data{
		"title1,title2,title3\n"
		"abc,def,ghi"
	};
	std::vector<CsvItem_t> items0{ {"title1"}, {"title2"}, {"title3"} };
	std::vector<CsvItem_t> items1{ {"abc"}, {"def"}, {"ghi"}, };
	std::vector<CsvLine_t> lines{
		{.items = {.list = items0.data(), .length = items0.size()}},
		{.items = {.list = items1.data(), .length = items1.size()}},
	};
	CsvContent_t content = { {.list = lines.data(), .length = lines.size()} };

	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromData(parser, data.c_str(), data.length()));

	AssertContent(&content);
}

TEST_F(CsvParserTest, Scenario2) {
	std::string data{
		"title1,title2\r\n"
		"data1,data2"
	};
	std::vector<CsvItem_t> items0{ {"title1"}, {"title2"} };
	std::vector<CsvItem_t> items1{ {"data1"}, {"data2"} };
	std::vector<CsvLine_t> lines{
		{.items = {.list = items0.data(), .length = items0.size()}},
		{.items = {.list = items1.data(), .length = items1.size()}},
	};
	CsvContent_t content = { {.list = lines.data(), .length = lines.size()} };

	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromData(parser, data.c_str(), data.length()));
	AssertContent(&content);
}

TEST_F(CsvParserTest, Scenario3) {
	std::string data{
		"title1,title2,title3\n"
		"data1"
	};
	std::vector<CsvItem_t> items0{ {"title1"}, {"title2"}, {"title3"} };
	std::vector<CsvItem_t> items1{ {"data1"} };
	std::vector<CsvLine_t> lines{
		{.items = {.list = items0.data(), .length = items0.size()}},
		{.items = {.list = items1.data(), .length = items1.size()}},
	};
	CsvContent_t content = { {.list = lines.data(), .length = lines.size()} };

	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromData(parser, data.c_str(), data.length()));
	AssertContent(&content);
}

TEST_F(CsvParserTest, Scenario4) {
	std::string data{ "" };
	std::vector<CsvItem_t> items{ {""} };
	std::vector<CsvLine_t> lines{
		{{.list = items.data(), .length = items.size()}}
	};
	CsvContent_t content = { {.list = lines.data(), .length = lines.size()} };

	ASSERT_EQ(CSV_INVALID_PARAMETER, CsvParser_LoadFromData(parser, data.c_str(), data.length()));
	// AssertContent(&content);
}

TEST_F(CsvParserTest, Scenario5) {
	/**
	 * input.csv
	 *  scenario5\r\n
	 *  title1,title2\r\n
	 *  data1,data2\r\n
	 *  EOF
	*/
	std::vector<CsvItem_t> items0{ {"scenario5"} };
	std::vector<CsvItem_t> items1{ {"title1"}, {"title2"} };
	std::vector<CsvItem_t> items2{ {"data1"}, {"data2"} };
	std::vector<CsvLine_t> lines{
		{{.list = items0.data(), .length = items0.size()}},
		{{.list = items1.data(), .length = items1.size()}},
		{{.list = items2.data(), .length = items2.size()}},
	};
	CsvContent_t content{ {.list = lines.data(), .length = lines.size()} };
	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromFile(parser, CSV_TEST_INPUT_FILE));
	AssertContent(&content);
}

TEST_F(CsvParserTest, Scenario6) {
	std::string data{
		"タイトル1,title2"
		"データ1,data2"
	};
	ASSERT_EQ(CSV_INVALID_CHAR_CODE, CsvParser_LoadFromData(parser, data.c_str(), data.length()));
}

TEST_F(CsvParserTest, NonAsciiByteInMiddleIsRejected) {
	// 0xFF は内部で番兵として使っているが、データ中に現れたら黙って打ち切らずエラーにする
	const char data[] = { 'a', ',', 'b', '\n', (char)0xFF, 'c', ',', 'd' };
	EXPECT_EQ(CSV_INVALID_CHAR_CODE, CsvParser_LoadFromData(parser, data, sizeof(data)));
}

TEST_F(CsvParserTest, LoadFromFileReadsWholeFile) {
	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromFile(parser, CSV_TEST_INPUT_FILE));
	const CsvContent_t *content = CsvParser_GetContent(parser);
	ASSERT_NE(nullptr, content);
	EXPECT_LT(0u, content->lines.length);
}

TEST_F(CsvParserTest, LoadFromMissingFileFails) {
	EXPECT_EQ(CSV_INVALID_PARAMETER, CsvParser_LoadFromFile(parser, "/nonexistent/path/input.csv"));
}

TEST_F(CsvParserTest, ManyLinesAndItems) {
	// 初期容量 (32) を超える行数・列数でも正しく解析できること
	std::string data;
	for (int line = 0; line < 100; line++) {
		for (int item = 0; item < 50; item++) {
			data += std::to_string(line * 100 + item);
			data += (item == 49) ? "\r\n" : ",";
		}
	}
	ASSERT_EQ(CSV_SUCCESS, CsvParser_LoadFromData(parser, data.c_str(), data.size()));
	const CsvContent_t *content = CsvParser_GetContent(parser);
	ASSERT_EQ(100u, content->lines.length);
	const CsvLine_t *last = &content->lines.list[99];
	ASSERT_EQ(50u, last->items.length);
	EXPECT_STREQ("9949", last->items.list[49].text);
}

// NOLINTEND
