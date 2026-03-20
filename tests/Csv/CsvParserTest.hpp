#pragma once

#include "gtest/gtest.h"
#include "Csv/parser/CsvParser.h"
#include "Csv/content/CsvContent.h"

// CMakeLists.txt の target_compile_definitions で設定される。
// IDEの静的解析用フォールバック。
#ifndef CSV_TEST_INPUT_FILE
#	define CSV_TEST_INPUT_FILE "input.csv"
#endif

class CsvParserTest : public::testing::Test {
protected:
	static constexpr CsvProperties_t props = { .hasHeader = false };
	CsvParser_t *parser;
	void SetUp() override;
	void TearDown() override;
	void AssertContent(CsvContent_t *expected);
};
