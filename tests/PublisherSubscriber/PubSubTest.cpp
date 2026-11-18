#include "PubSubTest.hpp"

void PubSubTest::SetUp() {}

void PubSubTest::TearDown() {}

TEST_F(PubSubTest, Publish) {
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	ASSERT_EQ(1, observer1.calledCount);
	ASSERT_EQ(1, observer1.publishes.size());
	EXPECT_EQ(subject.ATTR1, observer1.publishes[0].attribute);
	EXPECT_EQ(subject.MSG1, observer1.publishes[0].message);
}

TEST_F(PubSubTest, MultiplePublish) {
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	ASSERT_EQ(2, observer1.calledCount);
	EXPECT_EQ(subject.ATTR1, observer1.publishes[0].attribute);
	EXPECT_EQ(subject.MSG1, observer1.publishes[0].message);
	EXPECT_EQ(subject.ATTR1, observer1.publishes[1].attribute);
	EXPECT_EQ(subject.MSG1, observer1.publishes[1].message);
}

TEST_F(PubSubTest, Subscribers) {
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Subscribe(&observer2.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	ASSERT_EQ(1, observer1.calledCount);
	EXPECT_EQ(subject.ATTR1, observer1.publishes[0].attribute);
	EXPECT_EQ(subject.MSG1, observer1.publishes[0].message);
	ASSERT_EQ(1, observer2.calledCount);
	EXPECT_EQ(subject.ATTR1, observer2.publishes[0].attribute);
	EXPECT_EQ(subject.MSG1, observer2.publishes[0].message);
}

TEST_F(PubSubTest, DifferentAttribute) {
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR2);
	EXPECT_EQ(0, observer1.calledCount);
}

TEST_F(PubSubTest, Republish) {
	observer1.Update =
		[](Observer *observer, const PublishContent_t *content) {
		if (observer->calledCount <= 1) {
			return SUBSCRIBER_NACK;
		}
		return SUBSCRIBER_ACK;
	};
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	ASSERT_EQ(1, observer1.calledCount);
	subject.Publish(subject.MSG2, subject.ATTR2);
	EXPECT_EQ(2, observer1.calledCount);
}

TEST_F(PubSubTest, Unsubscribe) {
	SubscriptionAccountId id = subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Unsubscribe(id);
	subject.Publish(subject.MSG1, subject.ATTR1);
	EXPECT_EQ(0, observer1.calledCount);
}

// NACKで保留されたメッセージの内容が正しく再配信されることを確認
TEST_F(PubSubTest, RepublishContent) {
	observer1.Update =
		[](Observer *observer, const PublishContent_t *content) {
		if (observer->calledCount <= 1) {
			return SUBSCRIBER_NACK;
		}
		return SUBSCRIBER_ACK;
	};
	subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);
	ASSERT_EQ(1, observer1.calledCount);
	// ATTR2 のPublishで Republishがトリガーされ、MSG1 が再配信される
	subject.Publish(subject.MSG2, subject.ATTR2);
	ASSERT_EQ(2, observer1.calledCount);
	// 再配信されたのは元のMSG1であることを確認
	EXPECT_EQ(subject.MSG1, observer1.publishes[1].message);
	EXPECT_EQ(subject.ATTR1, observer1.publishes[1].attribute);
}

// Unsubscribe後、保留中のメッセージが破棄されることを確認
TEST_F(PubSubTest, PendingDroppedAfterUnsubscribe) {
	observer1.Update =
		[](Observer *observer, const PublishContent_t *content) {
		return SUBSCRIBER_NACK;	// 常にNACK（ペンディングに積み続ける）
	};
	SubscriptionAccountId id = subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);	// ペンディングに積まれる
	ASSERT_EQ(1, observer1.calledCount);
	subject.Unsubscribe(id);	// Unsubscribe後はペンディングも破棄される
	// 次のPublishでRepublishが走るがアカウントが無いのでペンディングは破棄される
	subject.Publish(subject.MSG2, subject.ATTR2);
	EXPECT_EQ(1, observer1.calledCount);	// 再配信されていない
}

// Unsubscribe後に残ったスロットへ新規Subscribeしても正しく配信されることを確認 (BUG-1の回帰テスト)
TEST_F(PubSubTest, PublishAfterUnsubscribeWithGap) {
	SubscriptionAccountId id1 = subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Subscribe(&observer2.subscriber, subject.ATTR1);
	subject.Unsubscribe(id1);	// observer1(id=0)を削除、スロット0が空になる
	subject.Subscribe(&observer3.subscriber, subject.ATTR1);	// スロット0に入る
	subject.Publish(subject.MSG1, subject.ATTR1);
	EXPECT_EQ(0, observer1.calledCount);	// Unsubscribe済みなので配信されない
	EXPECT_EQ(1, observer2.calledCount);	// 配信される
	EXPECT_EQ(1, observer3.calledCount);	// 配信される
}

// Unsubscribe 後に同じスロット(ID)へ別のサブスクライバーが入っても、
// 旧サブスクライバー宛ての保留メッセージが誤配信されないことを確認
TEST_F(PubSubTest, PendingNotDeliveredToNewSubscriberWithReusedId) {
	observer1.Update =
		[](Observer *observer, const PublishContent_t *content) {
		return SUBSCRIBER_NACK;	// 常にNACK（ペンディングに積み続ける）
	};
	SubscriptionAccountId id1 = subject.Subscribe(&observer1.subscriber, subject.ATTR1);
	subject.Publish(subject.MSG1, subject.ATTR1);	// observer1 宛てがペンディングに積まれる
	ASSERT_EQ(1, observer1.calledCount);
	subject.Unsubscribe(id1);
	SubscriptionAccountId id2 = subject.Subscribe(&observer2.subscriber, subject.ATTR2);
	ASSERT_EQ(id1, id2);	// 同じスロットが再利用される
	subject.Publish(subject.MSG2, subject.ATTR2);	// Republish が走る
	ASSERT_EQ(1, observer2.calledCount);	// 自分宛ての MSG2 のみ
	EXPECT_EQ(subject.MSG2, observer2.publishes[0].message);
}
