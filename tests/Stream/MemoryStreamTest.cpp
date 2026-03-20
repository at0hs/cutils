#include "MemoryStreamTest.hpp"
#include <cstring>
#include <vector>

MemoryStreamTest::MemoryStreamTest() {}

MemoryStreamTest::~MemoryStreamTest() {}

void MemoryStreamTest::SetUp() {
	ASSERT_EQ(MEMORY_STREAM_SUCCESS, MemoryStream_Init(&memoryStream));
}

void MemoryStreamTest::TearDown() {
	MemoryStream_Destroy(&memoryStream);
}

TEST_F(MemoryStreamTest, Init) {
	EXPECT_NE(nullptr, memoryStream.stream);
	EXPECT_NE(nullptr, memoryStream.buffer);
	EXPECT_EQ(0, memoryStream.position);
	EXPECT_EQ(0, memoryStream.size);
}

TEST_F(MemoryStreamTest, Write) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);
	ssize_t written = MemoryStream_Write(&memoryStream, data, dataSize);
	EXPECT_EQ(dataSize, (size_t)written);
	EXPECT_EQ(dataSize, memoryStream.size);
}

TEST_F(MemoryStreamTest, Read) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);
	MemoryStream_Write(&memoryStream, data, dataSize);
	ASSERT_EQ(MEMORY_STREAM_SUCCESS, MemoryStream_Seek(&memoryStream, 0, SEEK_SET));
	std::vector<char> buffer(dataSize + 1, 0);
	size_t bytesRead = MemoryStream_Read(&memoryStream, buffer.data(), dataSize);
	EXPECT_EQ(dataSize, bytesRead);
	EXPECT_STREQ(data, buffer.data());
}

TEST_F(MemoryStreamTest, Seek) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);
	MemoryStream_Write(&memoryStream, data, dataSize);
	ASSERT_EQ(MEMORY_STREAM_SUCCESS, MemoryStream_Seek(&memoryStream, 7, SEEK_SET));
	char buffer[6] = {0};
	size_t bytesRead = MemoryStream_Read(&memoryStream, buffer, 5);
	EXPECT_EQ(5, bytesRead);
	EXPECT_STREQ("World", buffer);
}

TEST_F(MemoryStreamTest, GetSize) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);
	MemoryStream_Write(&memoryStream, data, dataSize);
	size_t size = MemoryStream_GetSize(&memoryStream);
	EXPECT_EQ(dataSize, size);
}

TEST_F(MemoryStreamTest, SizeAfterSeek) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);

	MemoryStream_Write(&memoryStream, data, dataSize);
	MemoryStream_Seek(&memoryStream, -1, SEEK_CUR);
	MemoryStream_Write(&memoryStream, data, dataSize);

	size_t size = MemoryStream_GetSize(&memoryStream);
	size_t expected = dataSize * 2 - 1;

	EXPECT_EQ(expected, size);
}

TEST_F(MemoryStreamTest, RefBuffer) {
	const char* data = "Hello, World!";
	size_t dataSize = strlen(data);
	MemoryStream_Write(&memoryStream, data, dataSize);
	const void* buffer = MemoryStream_RefBuffer(&memoryStream);
	EXPECT_NE(nullptr, buffer);
}

TEST_F(MemoryStreamTest, NullSafety) {
	EXPECT_EQ(MEMORY_STREAM_INVALID_ARG, MemoryStream_Init(nullptr));
	MemoryStream_Destroy(nullptr);
	EXPECT_EQ(-1, MemoryStream_Write(nullptr, "x", 1));
	EXPECT_EQ(0U, MemoryStream_Read(nullptr, nullptr, 1));
	EXPECT_EQ(MEMORY_STREAM_INVALID_ARG, MemoryStream_Seek(nullptr, 0, SEEK_SET));
	EXPECT_EQ(nullptr, MemoryStream_RefBuffer(nullptr));
	EXPECT_EQ(0U, MemoryStream_GetSize(nullptr));
}
