#include "MessageStorageTest.hpp"

void MessageStorageTest::SetUp() {
	MessageStorage_Init(&messages, sizeof(std::string_view), maxMessage);
}

void MessageStorageTest::TearDown() {
	MessageStorage_Destroy(&messages);
}

MessageStorageReturnCode MessageStorageTest::Push(const std::string_view *adding) {
	return MessageStorage_Push(&messages, adding);
}

MessageStorageReturnCode MessageStorageTest::Pop(std::string_view *buffer) {
	return MessageStorage_Pop(&messages, buffer);
}

const std::string_view *MessageStorageTest::Peek() {
	return (const std::string_view *)MessageStorage_Peek(&messages);
}

void MessageStorageTest::MoveLast() {
	MessageStorage_MoveLast(&messages);
}

void MessageStorageTest::RemoveTop() {
	MessageStorage_RemoveTop(&messages);
}

TEST_F(MessageStorageTest, PushPoP) {
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg1));
	std::string_view str;
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&str));
	EXPECT_STREQ(msg1.data(), str.data());
}

TEST_F(MessageStorageTest, MultiplePushPop) {
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg1));
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg2));
	EXPECT_EQ(2, messages.count);
	std::string_view sv;
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&sv));
	EXPECT_STREQ(msg1.data(), sv.data());
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&sv));
	EXPECT_STREQ(msg2.data(), sv.data());
	EXPECT_EQ(0, messages.count);
}

TEST_F(MessageStorageTest, PeekAndMoveLast) {
	Push(&msg1);
	Push(&msg2);
	const std::string_view *sv = Peek();
	ASSERT_NE(nullptr, sv);
	EXPECT_STREQ(msg1.data(), sv->data());
	// MoveLast: 先頭(msg1)を末尾へ移動 → msg2, msg1 の順になる
	MoveLast();
	EXPECT_EQ(2, messages.count);
	std::string_view popped;
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&popped));
	EXPECT_STREQ(msg2.data(), popped.data());
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&popped));
	EXPECT_STREQ(msg1.data(), popped.data());
}

TEST_F(MessageStorageTest, RemoveTop) {
	Push(&msg1);
	Push(&msg2);
	RemoveTop();
	EXPECT_EQ(1, messages.count);
	std::string_view sv;
	Pop(&sv);
	EXPECT_STREQ(msg2.data(), sv.data());
}

TEST_F(MessageStorageTest, BufferEmpty) {
	std::string_view sv;
	EXPECT_EQ(MESSAGE_STORAGE_INVALID_ARG, Pop(nullptr));
	Push(&msg1);
	MoveLast();
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Pop(&sv));
	EXPECT_EQ(MESSAGE_STORAGE_EMPTY, Pop(&sv));
	EXPECT_EQ(nullptr, Peek());
	RemoveTop();
	EXPECT_EQ(0, messages.count);
}

TEST_F(MessageStorageTest, OverCapacity) {
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg1));
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg2));
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg3));
	ASSERT_EQ(MESSAGE_STORAGE_SUCCESS, Push(&msg4));
	ASSERT_EQ(MESSAGE_STORAGE_FULL, Push(&msg1));
}

TEST_F(MessageStorageTest, RoundTheArray) {
	Push(&msg1);
	Push(&msg2);
	Push(&msg3);
	Push(&msg4);
	std::string_view sv;
	Pop(&sv);
	ASSERT_EQ(msg1.data(), sv.data());
	Push(&msg5);
	Pop(&sv);
	ASSERT_STREQ(msg2.data(), sv.data());
	Pop(&sv);
	ASSERT_STREQ(msg3.data(), sv.data());
	Pop(&sv);
	ASSERT_STREQ(msg4.data(), sv.data());
	Pop(&sv);
	ASSERT_STREQ(msg5.data(), sv.data());
}
