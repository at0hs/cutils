/**
 * @file Publisher.h
 * @brief パブリッシャー
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Broker.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief パブリッシャー制御ブロック
 * @details Broker をラップする公開APIの中心的な型。
 *          サブスクライバーの登録・解除、メッセージの配信を提供する。
 *          Broker の詳細（ペンディング再配信など）はこの型の内部に隠蔽される。
 */
typedef struct Publisher_t {
  //! @name Private
  //! @{

  //! 仲介役ブローカー
  Broker_t broker;

  //! @}
} Publisher_t;

/**
 * @brief パブリッシャーを初期化する
 * @param self           初期化対象のインスタンス（NULL 不可）
 * @param maxSubscribers 登録可能なサブスクライバーの最大数
 */
void Publisher_Init(Publisher_t *self, uint8_t maxSubscribers);

/**
 * @brief サブスクライバーを登録する
 * @param self              インスタンス（NULL の場合は INVALID_ACCOUNT_ID を返す）
 * @param subscriber        登録するサブスクライバー情報（NULL 不可）
 * @param interestedPublish 購読するメッセージの属性
 * @return 割り当てられたアカウントID。登録失敗時は INVALID_ACCOUNT_ID。
 */
SubscriptionAccountId
Publisher_Subscribe(Publisher_t *self, const Subscriber_t *subscriber,
                    PublishMessageAttribute interestedPublish);

/**
 * @brief サブスクライバーの登録を解除する
 * @param self インスタンス（NULL の場合は何もしない）
 * @param id   解除するアカウントID
 */
void Publisher_Unsubscribe(Publisher_t *self, SubscriptionAccountId id);

/**
 * @brief メッセージを配信する
 * @param self    インスタンス（NULL の場合は何もしない）
 * @param publish 配信するメッセージ内容（NULL の場合は何もしない）
 */
void Publisher_Publish(Publisher_t *self, const PublishContent_t *publish);

/**
 * @brief パブリッシャーを破棄する
 * @param self インスタンス（NULL の場合は何もしない）
 */
void Publisher_Destroy(Publisher_t *self);
#ifdef __cplusplus
}
#endif
