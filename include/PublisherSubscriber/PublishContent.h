/**
 * @file PublishContent.h
 * @brief パブリッシュの内容
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @typedef PublishMessageAttribute
 * @brief メッセージの属性を表す型
 * @details 購読フィルタリングに使用される整数値。
 *          Publisher/Subscriber 双方で一致した場合にメッセージが配信される。
 */
typedef int PublishMessageAttribute;

/**
 * @typedef PublishMessage
 * @brief メッセージ本体を表す型
 * @details 64ビット整数として表現されるメッセージ値。
 *          ポインタ値またはスカラー値を格納するために使用される。
 */
typedef uint64_t PublishMessage;

/**
 * @brief パブリッシュの内容
 * @details Broker を介してサブスクライバーへ配信されるデータ単位。
 *          メッセージ本体と属性（フィルタキー）を保持する。
 */
typedef struct PublishContent_t {
  //! メッセージ本体
  PublishMessage message;
  //! メッセージの属性（購読フィルタキー）
  PublishMessageAttribute attribute;
} PublishContent_t;

#ifdef __cplusplus
}
#endif
