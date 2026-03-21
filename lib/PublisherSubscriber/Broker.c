/**
 * @file Broker.c
 * @brief パブリッシャーとサブスクライバーの仲介
 * @author atohs
 * @date 2024/07/12
 */
#include "PublisherSubscriber/Broker.h"
#include "PublisherSubscriber/MessageStorage.h"
#include "PublisherSubscriber/Subscriber.h"
#include "PublisherSubscriber/Subscription.h"
#include "utilities.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief ペンディングキューに保留された配信情報
 * @details NACK を返したサブスクライバー宛のメッセージをキューに積む際に使用する。
 *          アカウントIDとメッセージ内容をペアで保持することで、
 *          再配信時に正しい宛先へ届けられる。
 */
typedef struct Delivery_t {
  //! 配信先のアカウントID
  SubscriptionAccountId id;
  //! 配信するメッセージ内容
  PublishContent_t content;
} Delivery_t;

/**
 * @brief ペンディングキューに保留されたメッセージを再配信する
 * @details 関数呼び出し時点のキュー件数分だけ再配信を試みる（無限ループ防止）。
 *          - ACK を返した場合: キューから削除する
 *          - NACK を返した場合: キューの末尾に移動して次回に持ち越す
 *          - 宛先アカウントが存在しない（Unsubscribe済み）場合: 破棄する
 * @param self インスタンス
 */
static void Republish(Broker_t *self) {
  MessageStorage_t *pending = &self->pendingMessages;
  Subscription_t *subscription = &self->subscription;
  // 保留中のメッセージを一回だけ送りたいので、最初のたまっている数を覚えておく
  size_t count = pending->count;

  for (size_t i = 0; i < count; i++) {
    const Delivery_t *delivery = MessageStorage_Peek(pending);
    if (UNLIKELY(!delivery)) {
      break;
    }

    SubscriptionAccount_t *account =
        Subscription_GetAccount(subscription, delivery->id);
    if (UNLIKELY(!account)) {
      // Unsubscribe済みのアカウント宛メッセージは破棄する
      MessageStorage_RemoveTop(pending);
      continue;
    }
    if (Subscriber_Update(&account->subscriber, &delivery->content) ==
        SUBSCRIBER_NACK) {
      MessageStorage_MoveLast(pending);  // NACKの場合は末尾に移動して再配信待ち
    } else {
      MessageStorage_RemoveTop(pending);  // ACKの場合のみ削除
    }
  }
}

/**
 * @brief 初期化
 * @param self           インスタンス（NULL の場合は何もしない）
 * @param maxSubscribers 登録可能なサブスクライバーの最大数
 *                       ペンディングキューの容量は maxSubscribers × 16 になる
 */
void Broker_Init(Broker_t *self, uint8_t maxSubscribers) {
  if (UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);
  self->maxSubscribers = maxSubscribers;
  Subscription_Init(&self->subscription, maxSubscribers);
  MessageStorage_Init(
      &self->pendingMessages, sizeof(Delivery_t), maxSubscribers * 16UL);
}

/**
 * @brief 全マッチサブスクライバーへメッセージを配信する
 * @details 配信前にペンディングキューの再配信を行う。
 *          NACK を返したサブスクライバー宛メッセージはキューに積む。
 *          キュー満杯の場合はそのメッセージは失われる（設計上の制約）。
 * @param self    インスタンス（NULL の場合は何もしない）
 * @param content 配信するメッセージ内容（NULL の場合は何もしない）
 */
void Broker_Publish(Broker_t *self, const PublishContent_t *content) {
  if (UNLIKELY(!self || !content)) {
    return;
  }
  Republish(self);
  if (UNLIKELY(self->maxSubscribers == 0)) {
    return;
  }
  SubscriptionAccountId ids[self->maxSubscribers];
  int numIds = Subscription_Match(
      &self->subscription, content->attribute, ids, self->maxSubscribers);
  if (numIds <= 0) {
    return;
  }
  for (size_t i = 0; i < (size_t)numIds; i++) {
    SubscriptionAccount_t *account =
        Subscription_GetAccount(&self->subscription, ids[i]);
    if (UNLIKELY(account == NULL)) {
      continue;
    }
    if (Subscriber_Update(&account->subscriber, content) == SUBSCRIBER_NACK) {
      // キュー満杯の場合はメッセージを失う（設計上の制約）
      (void)MessageStorage_Push(
          &self->pendingMessages,
          &(Delivery_t){.id = account->id, .content = *content});
    }
  }
}

/**
 * @brief サブスクライバーを登録する
 * @param self            インスタンス
 * @param subscriber      登録するサブスクライバー情報
 * @param interestedTopic 購読するメッセージの属性
 * @return 割り当てられたアカウントID。失敗時は INVALID_ACCOUNT_ID。
 */
SubscriptionAccountId Broker_Subscribe(
    Broker_t *self,
    const Subscriber_t *subscriber,
    PublishMessageAttribute interestedTopic) {
  return Subscription_Contract(
      &self->subscription, subscriber, interestedTopic);
}

/**
 * @brief サブスクライバーの登録を解除する
 * @note ペンディングキュー内の該当メッセージはここでは削除しない。
 *       Republish 時にアカウントが見つからなかった際に破棄する遅延削除方式を採る。
 * @param self インスタンス（NULL の場合は何もしない）
 * @param id   解除するアカウントID
 */
void Broker_Unsubscribe(Broker_t *self, SubscriptionAccountId id) {
  /**
   * @note ペンディングしているメッセージはここでは消さず、
   * Republish するときにアカウントが見つからなければ消すという方法を取る
   */
  Subscription_Cancellation(&self->subscription, id);
}

/**
 * @brief インスタンスを破棄
 * @param self インスタンス
 */
void Broker_Destroy(Broker_t *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  Subscription_Destroy(&self->subscription);
  MessageStorage_Destroy(&self->pendingMessages);
}
