/**
 * @file Subscription.h
 * @brief サブスクリプション
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Subscriber.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @typedef SubscriptionAccountId
 * @brief サブスクリプションのアカウントID
 * @details Subscription_Contract が返すインデックス値。
 *          INVALID_ACCOUNT_ID はエラーまたは未登録を示す。
 */
typedef size_t SubscriptionAccountId;

/**
 * @def INVALID_ACCOUNT_ID
 * @brief 無効なアカウントIDを示すセンチネル値
 * @details Subscription_Contract の失敗時に返される。
 *          登録上限超過またはNULLポインタ渡しが原因となる。
 */
#define INVALID_ACCOUNT_ID ((SubscriptionAccountId) - 1)

/**
 * @brief 個別サブスクリプションのアカウント情報
 * @details Subscription_t が内部で管理するエントリ。
 *          contracted が false のスロットは空き状態を表す。
 */
typedef struct SubscriptionAccount_t {
  //! このアカウントに割り当てられたID（配列インデックスと一致）
  SubscriptionAccountId id;
  //! 紐付けられたサブスクライバー情報
  Subscriber_t subscriber;
  //! このアカウントが購読するメッセージ属性
  PublishMessageAttribute interestedPublish;
  //! true: 有効な契約済みアカウント / false: 空きスロット
  bool contracted;
} SubscriptionAccount_t;

/**
 * @brief サブスクリプション管理の制御ブロック
 * @details 複数の SubscriptionAccount_t を配列で保持し、
 *          購読の登録・解除・マッチングを担う。
 */
typedef struct Subscription_t {
  //! @name Private
  //! @{

  //! アカウントの配列（動的確保）
  SubscriptionAccount_t *accounts;
  //! 登録可能な最大アカウント数
  size_t numAccounts;

  //! @}
} Subscription_t;

/**
 * @brief サブスクリプションを初期化する
 * @param self        初期化対象のインスタンス（NULL 不可）
 * @param numAccounts 登録可能なサブスクライバーの最大数
 */
void Subscription_Init(Subscription_t *self, size_t numAccounts);

/**
 * @brief サブスクライバーを登録して購読を開始する
 * @param self             インスタンス（NULL 不可）
 * @param subscriber       登録するサブスクライバー情報（NULL 不可）
 * @param interestedPublish 購読するメッセージの属性
 * @return 割り当てられたアカウントID。登録失敗時は INVALID_ACCOUNT_ID。
 */
SubscriptionAccountId Subscription_Contract(
    Subscription_t *self,
    const Subscriber_t *subscriber,
    PublishMessageAttribute interestedPublish);

/**
 * @brief 購読を解除してアカウントを無効化する
 * @param self インスタンス（NULL または範囲外IDの場合は何もしない）
 * @param id   解除するアカウントID
 */
void Subscription_Cancellation(Subscription_t *self, SubscriptionAccountId id);

/**
 * @brief アカウントIDに対応する有効なアカウントを取得する
 * @param self インスタンス
 * @param id   取得するアカウントID
 * @return 有効なアカウントへのポインタ。未登録・範囲外・NULL の場合は NULL。
 */
SubscriptionAccount_t *Subscription_GetAccount(Subscription_t *self,
                                               SubscriptionAccountId id);

/**
 * @brief 指定した属性と一致する全アカウントのIDを取得する
 * @param self             インスタンス
 * @param messageAttribute マッチさせるメッセージ属性
 * @param matchedIds       マッチしたIDを格納する出力バッファ
 * @param size             バッファの要素数
 * @return マッチしたアカウント数。エラー（NULL・バッファ不足）の場合は -1。
 */
int Subscription_Match(Subscription_t *self,
                       PublishMessageAttribute messageAttribute,
                       SubscriptionAccountId matchedIds[],
                       size_t size);

/**
 * @brief サブスクリプションを破棄する
 * @param self インスタンス（NULL の場合は何もしない）
 */
void Subscription_Destroy(Subscription_t *self);

/**
 * @brief 現在契約中のアカウント数を返す
 * @param self インスタンス
 * @return 有効なアカウント数。NULL の場合は 0。
 */
size_t Subscription_Count(Subscription_t *self);

#ifdef __cplusplus
}
#endif
