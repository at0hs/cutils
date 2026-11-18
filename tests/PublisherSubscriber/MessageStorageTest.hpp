#pragma once

#include "gtest/gtest.h"
#include "PublisherSubscriber/MessageStorage.h"
#include <string_view>

class MessageStorageTest : public ::testing::Test {
protected:
	static constexpr std::string_view msg1 = "MESSAGE1";
	static constexpr std::string_view msg2 = "MESSAGE2";
	static constexpr std::string_view msg3 = "MESSAGE3";
	static constexpr std::string_view msg4 = "MESSAGE4";
	static constexpr std::string_view msg5 = "MESSAGE5";

	MessageStorage_t messages;
	static constexpr int maxMessage = 4;
	virtual void SetUp();
	virtual void TearDown();

	MessageStorageReturnCode Push(const std::string_view *adding);
	MessageStorageReturnCode Pop(std::string_view *buffer);
	const std::string_view *Peek();
	void MoveLast();
	void RemoveTop();
};
