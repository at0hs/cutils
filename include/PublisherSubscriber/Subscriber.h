/**
 * @file Subscriber.h
 * @brief サブスクライバー
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "PublishContent.h"

/**
 * @brief サブスクライバーの応答コード
 * @details Broker はこの返り値に基づいてメッセージの再配信を制御する。
 *          NACK を返すとメッセージはペンディングキューに積まれ、
 *          次回の Publish 時に再配信が試みられる。
 */
typedef enum SubscriberReply {
  SUBSCRIBER_ACK = 0, //!< 受信成功（メッセージはキューから削除される）
  SUBSCRIBER_NACK,    //!< 受信失敗（メッセージはキューに保留される）
} SubscriberReply;

/**
 * @typedef SubscriberUpdate
 * @brief メッセージ受信時に呼び出されるコールバック関数型
 * @param publish 受信したパブリッシュの内容（NULLの場合あり）
 * @param userData Subscriber_Init で登録したユーザーデータ
 * @return SUBSCRIBER_ACK: 正常受信 / SUBSCRIBER_NACK: 再配信要求
 */
typedef SubscriberReply (*SubscriberUpdate)(const PublishContent_t *publish,
                                            void *userData);

/**
 * @struct Subscriber_t
 * @brief サブスクライバー制御ブロック
 * @details メッセージ受信コールバックとユーザーデータを保持する。
 *          Subscription に登録することで Broker 経由のメッセージを受け取る。
 */
typedef struct Subscriber_t {
  //! @name Private
  //! @{

  //! メッセージ受信時に呼ばれるコールバック
  SubscriberUpdate update;
  //! コールバックに透過渡しするユーザーデータ
  void *userData;

  //! @}
} Subscriber_t;

/**
 * @brief サブスクライバーを初期化する
 * @param self  初期化対象のインスタンス（NULL 不可）
 * @param update メッセージ受信コールバック（NULL の場合は常に ACK を返す）
 * @param userData コールバックに渡す任意のユーザーデータ（NULL 許容）
 */
void Subscriber_Init(Subscriber_t *self, SubscriberUpdate update,
                     void *userData);

/**
 * @brief サブスクライバーへメッセージを通知する
 * @param self    インスタンス（NULL の場合は NACK を返す）
 * @param publish 配信するメッセージ内容
 * @return コールバックの返り値。コールバック未設定の場合は SUBSCRIBER_ACK。
 */
SubscriberReply Subscriber_Update(Subscriber_t *self,
                                  const PublishContent_t *publish);

/**
 * @brief サブスクライバーを破棄する
 * @param self インスタンス（NULL の場合は何もしない）
 */
void Subscriber_Destroy(Subscriber_t *self);
#ifdef __cplusplus
}
#endif
