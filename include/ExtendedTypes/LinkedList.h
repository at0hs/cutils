/**
 * @file LinkedList.h
 * @brief 単方向リスト
 * @author atohs
 * @date 2024/08/05
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ExtendedTypes/MemoryAllocator.h"

/**
 * @brief リストノード
 */
typedef struct LinkedListNode_t {
  //! @name private
  //! @{

  void *object;                  /*!< 格納オブジェクトへのポインタ */
  struct LinkedListNode_t *next; /*!< 次のノードへのポインタ（末尾は NULL） */

  //! @}
} LinkedListNode_t;

/**
 * @brief 単方向リスト制御ブロック
 */
typedef struct LinkedList_t {
  //! @name private
  //! @{

  LinkedListNode_t *head;      /*!< 先頭ノードへのポインタ（空の場合は NULL） */
  size_t objectSize;           /*!< 格納オブジェクトのサイズ（バイト） */
  MemoryAllocator_t allocator; /*!< オブジェクト用メモリアロケーター */
  size_t count;                /*!< 現在の要素数 */

  //! @}
} LinkedList_t;

/**
 * @brief 先頭ノードを取得する
 * @param[in] self リストインスタンス
 * @return 先頭ノードへのポインタ（空の場合は NULL）
 */
static inline const LinkedListNode_t *LinkedList_GetRoot(
    const LinkedList_t *self) {
  return self->head;
}

/**
 * @brief 次のノードを取得する
 * @param[in] self ノードインスタンス
 * @return 次のノードへのポインタ（末尾の場合は NULL）
 */
static inline const LinkedListNode_t *LinkedList_GetNext(
    const LinkedListNode_t *self) {
  return self->next;
}

/**
 * @brief ノードに格納されたオブジェクトを取得する
 * @param[in] self ノードインスタンス
 * @return オブジェクトへのポインタ
 */
static inline const void *LinkedList_GetObject(const LinkedListNode_t *self) {
  return self->object;
}

/**
 * @brief リストの全要素を順に巡回するループマクロ
 * @param self リストインスタンス (`LinkedList_t *`)
 * @param node 各反復で現在のノードを受け取る変数 (`const LinkedListNode_t *`)
 * @note break / continue が使用できる
 */
#define LinkedList_Foreach(self, node)            \
  for ((node) = LinkedList_GetRoot(self); (node); \
       (node) = LinkedList_GetNext(node))

//! @name constructor
//! @{

/**
 * @brief LinkedList を初期化する
 * @param[out] self        リストインスタンス
 * @param[in]  allocator   メモリアロケーター（NULL の場合はデフォルト malloc/free を使用）
 * @param[in]  objectSize  格納するオブジェクトのサイズ（バイト）
 */
extern void LinkedList_Init(LinkedList_t *self,
                            const MemoryAllocator_t *allocator,
                            size_t objectSize);

//! @}

//! @name destructor
//! @{

/**
 * @brief LinkedList を破棄する
 * 全要素を解放し、制御ブロックをゼロクリアする
 * @param[in,out] self リストインスタンス
 */
extern void LinkedList_Destroy(LinkedList_t *self);
//! @}

//! @name properties
//! @{
/**
 * @brief リストに格納されている要素数を取得する
 * @param[in] self リストインスタンス
 * @return 要素数
 */
extern size_t LinkedList_Count(LinkedList_t *self);

/**
 * @brief リストが空かどうかを判定する
 * @param[in] self リストインスタンス
 * @return 空の場合は true、それ以外は false
 */
extern bool LinkedList_IsEmpty(LinkedList_t *self);
//! @}

//! @name methods
//! @{
/**
 * @brief リストの末尾に要素を追加する（ディープコピー）
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 追加するオブジェクトへのポインタ
 */
extern void LinkedList_Add(LinkedList_t *self, const void *object);

/**
 * @brief 指定されたインデックスの要素を取得する
 * @param[in] self  リストインスタンス
 * @param[in] index インデックス（0 始まり）
 * @return 要素へのポインタ。範囲外の場合は NULL
 */
extern const void *LinkedList_At(LinkedList_t *self, uint32_t index);

/**
 * @brief リストに特定のオブジェクトが含まれているか確認する
 * メモリ内容の比較（memcmp）により判定する
 * @param[in] self   リストインスタンス
 * @param[in] object 検索するオブジェクト
 * @return 含まれている場合は true、それ以外は false
 */
extern bool LinkedList_Contains(LinkedList_t *self, void *object);

/**
 * @brief 指定されたオブジェクトをリストから削除する
 * 最初に見つかった要素のみを削除する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 削除するオブジェクト
 * @return 削除に成功した場合は true、要素が見つからない場合は false
 */
extern bool LinkedList_Remove(LinkedList_t *self, const void *object);

/**
 * @brief 指定されたインデックスの要素を削除する
 * @param[in,out] self  リストインスタンス
 * @param[in]     index 削除するインデックス（0 始まり）
 * @return 削除に成功した場合は true、インデックスが範囲外の場合は false
 */
extern bool LinkedList_RemoveAt(LinkedList_t *self, uint32_t index);

/**
 * @brief リストのすべての要素を削除する
 * @param[in,out] self リストインスタンス
 */
extern void LinkedList_Clear(LinkedList_t *self);

//! @}

#ifdef __cplusplus
}
#endif
