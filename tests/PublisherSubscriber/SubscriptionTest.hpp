#pragma once

#include "gtest/gtest.h"
#include "PublisherSubscriber/Subscription.h"

class SubscriptionTest : public ::testing::Test {
protected:
	static constexpr int attr1 = 1;
	static constexpr int attr2 = 2;
	static constexpr int attr3 = 3;
	static constexpr int attr4 = 4;

	Subscription_t subscription;
	static constexpr int numAccounts = 4;
	virtual void SetUp();
	virtual void TearDown();

	SubscriptionAccountId Contract(Subscriber_t *subscriber, PublishMessageAttribute attribute);
	void Cancellation(SubscriptionAccountId id);
	int Match(PublishMessageAttribute attribute, SubscriptionAccountId matchedIds[], size_t bufferSize);
	size_t Count();
	SubscriptionAccount_t *GetAccount(SubscriptionAccountId id);
	static SubscriberReply DummyUpdate(const PublishContent_t *publish, void *arg);
};
