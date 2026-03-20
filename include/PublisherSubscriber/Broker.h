/**
 * @file Broker.h
 * @brief ブローカー
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "MessageStorage.h"
#include "PublishContent.h"
#include "Subscriber.h"
#include "Subscription.h"
/**
 * @brief ブローカー制御ブロック
 * @details Publisher と Subscriber の仲介役。
 *          Subscription によるフィルタリングと、
 *          MessageStorage を用いたペンディングメッセージの再配信を管理する。
 *
 *          配信フロー:
 *          1. Broker_Publish 呼び出し時にまずペンディングキューの再配信を試みる
 *          2. 属性が一致するサブスクライバーへメッセージを配信する
 *          3. NACK を返したサブスクライバー宛のメッセージはペンディングキューに積む
 */
typedef struct Broker_t {
  //! @name Private
  //! @{

  //! サブスクリプション管理
  Subscription_t subscription;
  //! NACK されたメッセージを保留する循環キュー
  MessageStorage_t pendingMessages;
  //! 登録可能なサブスクライバーの最大数
  uint8_t maxSubscribers;

  //! @}
} Broker_t;

/**
 * @brief ブローカーを初期化する
 * @param self           初期化対象のインスタンス（NULL 不可）
 * @param maxSubscribers 登録可能なサブスクライバーの最大数
 *                       （ペンディングキューの容量は maxSubscribers × 16 になる）
 */
void Broker_Init(Broker_t *self, uint8_t maxSubscribers);

/**
 * @brief メッセージを全マッチサブスクライバーへ配信する
 * @details 呼び出し前にペンディングキューの再配信を試みる。
 *          NACK を返したサブスクライバー宛メッセージはペンディングキューに保留する。
 *          キュー満杯の場合はそのメッセージは失われる。
 * @param self    インスタンス（NULL の場合は何もしない）
 * @param content 配信するメッセージ内容（NULL の場合は何もしない）
 */
void Broker_Publish(Broker_t *self, const PublishContent_t *content);

/**
 * @brief サブスクライバーを登録する
 * @param self               インスタンス（NULL の場合は INVALID_ACCOUNT_ID を返す）
 * @param subscriber         登録するサブスクライバー情報（NULL 不可）
 * @param interestedPublish  購読するメッセージの属性
 * @return 割り当てられたアカウントID。登録失敗時は INVALID_ACCOUNT_ID。
 */
SubscriptionAccountId
Broker_Subscribe(Broker_t *self, const Subscriber_t *subscriber,
                 PublishMessageAttribute interestedPublish);

/**
 * @brief サブスクライバーの登録を解除する
 * @note ペンディングキュー内の該当メッセージは即座に削除されず、
 *       次回の再配信試行時にアカウントが見つからない場合に破棄される。
 * @param self インスタンス（NULL の場合は何もしない）
 * @param id   解除するアカウントID
 */
void Broker_Unsubscribe(Broker_t *self, SubscriptionAccountId id);

/**
 * @brief ブローカーを破棄する
 * @param self インスタンス（NULL の場合は何もしない）
 */
void Broker_Destroy(Broker_t *self);

#ifdef __cplusplus
}
#endif
