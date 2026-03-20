/**
 * @file MessageStorage.h
 * @brief メッセージストレージ
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief メッセージストレージ制御ブロック
 * @details 固定サイズ要素の循環キュー（リングバッファ）実装。
 *          Broker がペンディングメッセージの保留と再配信に使用する。
 *          メッセージサイズは初期化時に固定され、異なるサイズの混在は不可。
 */
typedef struct MessageStorage_t {
  //! @name Private
  //! @{

  //! メッセージを格納する動的確保バッファ
  void *messages;
  //! 1メッセージあたりのバイトサイズ
  size_t messageSize;
  //! 現在格納されているメッセージ数
  size_t count;
  //! バッファの最大メッセージ格納数
  size_t capacity;
  //! デキュー側（先頭）インデックス
  size_t top;
  //! エンキュー側（末尾）インデックス
  size_t bottom;

  //! @}
} MessageStorage_t;

/**
 * @brief メッセージストレージを初期化する
 * @param self        初期化対象のインスタンス（NULL 不可）
 * @param messageSize 1要素のバイトサイズ
 * @param capacity    格納可能な最大メッセージ数
 */
void MessageStorage_Init(MessageStorage_t *self, size_t messageSize,
                         size_t capacity);

/**
 * @brief メッセージをキューの末尾に追加する
 * @param self   インスタンス（NULL の場合は -1 を返す）
 * @param adding 追加するデータへのポインタ（messageSize バイト分コピーされる）
 * @return 0: 成功 / -1: 失敗（NULL・キュー満杯）
 */
int MessageStorage_Push(MessageStorage_t *self, const void *adding);

/**
 * @brief キューの先頭からメッセージを取り出す
 * @param self   インスタンス（NULL の場合は -1 を返す）
 * @param buffer 取り出したデータを格納するバッファ（messageSize バイト以上必要）
 * @return 0: 成功 / -1: 失敗（NULL・キュー空）
 */
int MessageStorage_Pop(MessageStorage_t *self, void *buffer);

/**
 * @brief キューの先頭メッセージへの参照を返す（取り出しは行わない）
 * @param self インスタンス
 * @return 先頭メッセージへの読み取り専用ポインタ。空またはNULLの場合は NULL。
 */
const void *MessageStorage_Peek(MessageStorage_t *self);

/**
 * @brief キューの先頭メッセージを末尾へ移動する
 * @details NACK 時の再配信制御に使用する。count が 1 以下の場合は何もしない。
 * @param self インスタンス（NULL の場合は何もしない）
 */
void MessageStorage_MoveLast(MessageStorage_t *self);

/**
 * @brief キューの先頭メッセージを破棄する
 * @details Pop と異なりデータのコピーを行わない。
 * @param self インスタンス（NULL またはキュー空の場合は何もしない）
 */
void MessageStorage_RemoveTop(MessageStorage_t *self);

/**
 * @brief メッセージストレージを破棄する
 * @param self インスタンス（NULL の場合は何もしない）
 */
void MessageStorage_Destroy(MessageStorage_t *self);
#ifdef __cplusplus
}
#endif
