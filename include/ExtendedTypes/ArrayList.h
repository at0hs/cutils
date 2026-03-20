/**
 * @file ArrayList.h
 * @brief リスト
 * @author atohs
 * @date 2024/08/06
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ExtendedTypes/MemoryAllocator.h"

/**
 * @brief ノード
 */
typedef struct ArrayListNode {
  //! @name private
  //! @{
  void *object;
  bool hadEntity;
  //! @}
} ArrayListNode;

/**
 * @brief 制御ブロック
 */
typedef struct ArrayList {
  //! @name private
  //! @{
  ArrayListNode *list;
  size_t objectSize;
  size_t capacity;
  size_t length;
  MemoryAllocator_t allocator;
  //! @}
} ArrayList;

//! @name constructor
//! @{
/**
 * @brief ArrayListを初期化する
 * @param[out] self       リストインスタンス
 * @param[in]  allocator  メモリアロケータ（NULLの場合はデフォルト malloc/freeを使用）
 * @param[in]  objectSize 格納するオブジェクトのサイズ（バイト）
 */
extern void ArrayList_Init(ArrayList *self, const MemoryAllocator_t *allocator,
                           size_t objectSize);
//! @}

//! @name destructor
//! @{
/**
 * @brief ArrayListを破棄する
 * 確保されたすべてのリソースを解放する
 * @param[in,out] self リストインスタンス
 */
extern void ArrayList_Destroy(ArrayList *self);
//! @}

//! @name properties
//! @{
/**
 * @brief 現在の容量を取得する
 * @param[in] self リストインスタンス
 * @return 容量（確保済みスロット数）
 */
extern size_t ArrayList_Capacity(const ArrayList *self);

/**
 * @brief 格納されている要素数を取得する
 * @param[in] self リストインスタンス
 * @return 要素数
 */
extern size_t ArrayList_Length(const ArrayList *self);
//! @}

//! @name methods
//! @{
/**
 * @brief リストが空かどうかを判定する
 * @param[in] self リストインスタンス
 * @return 空の場合は true、それ以外は false
 */
extern bool ArrayList_IsEmpty(const ArrayList *self);

/**
 * @brief リストの容量を変更する
 * @param[in,out] self    リストインスタンス
 * @param[in]     newSize 新しい容量
 */
extern void ArrayList_Resize(ArrayList *self, size_t newSize);

/**
 * @brief 指定されたインデックスの要素を取得する
 * @param[in] self  リストインスタンス
 * @param[in] index インデックス
 * @return 要素へのポインタ。範囲外の場合は NULL
 */
extern const void *ArrayList_At(ArrayList *self, uint64_t index);

/**
 * @brief リストの末尾に要素を追加する（ディープコピー）
 * 必要に応じて容量を自動的に拡張する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 追加するオブジェクト
 */
extern void ArrayList_Add(ArrayList *self, const void *object);

/**
 * @brief リストの末尾に要素の参照を追加する
 * オブジェクトはコピーされず、ポインタのみが保持される
 * 必要に応じて容量を自動的に拡張する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 追加するオブジェクトへのポインタ
 */
extern void ArrayList_AddReference(ArrayList *self, void *object);

/**
 * @brief 複数の要素をリストの末尾に追加する
 * @param[in,out] self    リストインスタンス
 * @param[in]     objects 追加するオブジェクトの配列
 * @param[in]     length  追加する要素数
 */
extern void ArrayList_AddRange(ArrayList *self, const void *objects,
                               size_t length);

/**
 * @brief 別のリストを現在のリストの末尾に結合する
 * 追加されるリストの各要素の管理状態（コピーか参照か）を引き継ぐ
 * @param[in,out] self リストインスタンス
 * @param[in]     list 追加するリスト
 */
extern void ArrayList_Append(ArrayList *self, const ArrayList *list);

/**
 * @brief 指定された位置に要素を挿入する（ディープコピー）
 * 以降の要素は一つ後ろにシフトされる
 * @param[in,out] self   リストインスタンス
 * @param[in]     index  挿入位置（既存要素の範囲内）
 * @param[in]     object 挿入するオブジェクト
 */
extern void ArrayList_Insert(ArrayList *self, uint64_t index,
                             const void *object);

/**
 * @brief 指定されたオブジェクトのインデックスを検索する
 * メモリ内容の比較（memcmp）により判定する
 * @param[in] self   リストインスタンス
 * @param[in] object 検索するオブジェクト
 * @return 見つかったインデックス。見つからない場合は -1
 */
extern ssize_t ArrayList_IndexOf(ArrayList *self, const void *object);

/**
 * @brief リストに特定のオブジェクトが含まれているか確認する
 * @param[in] self   リストインスタンス
 * @param[in] object 検索するオブジェクト
 * @return 含まれている場合は true、それ以外は false
 */
extern bool ArrayList_Contains(ArrayList *self, const void *object);

/**
 * @brief 指定されたオブジェクトをリストから削除する
 * 最初に見つかった要素のみを削除し、以降の要素を詰める
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 削除するオブジェクト
 * @return 削除に成功した場合は true、要素が見つからない場合は false
 */
extern bool ArrayList_Remove(ArrayList *self, const void *object);

/**
 * @brief 指定されたインデックスの要素を削除する
 * 以降の要素は一つ前にシフトされる
 * @param[in,out] self  リストインスタンス
 * @param[in]     index 削除するインデックス
 * @return 削除に成功した場合は true、インデックスが範囲外の場合は false
 */
extern bool ArrayList_RemoveAt(ArrayList *self, uint64_t index);

/**
 * @brief リストのすべての要素を削除する
 * @param[in,out] self リストインスタンス
 */
extern void ArrayList_Clear(ArrayList *self);

/**
 * @brief リストの各要素に対してコールバック関数を実行する
 * @param[in] self     リストインスタンス
 * @param[in] callback
 * 各要素に対して実行される関数。引数はインデックスと要素へのポインタ
 */
extern void ArrayList_ForEach(ArrayList *self,
                              void (*callback)(uint64_t index,
                                               const void *object));
//! @}
#ifdef __cplusplus
}
#endif
