
/**
 * @file CsvItemCollection.h
 * @brief アイテムリスト
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "CsvItem.h"
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief 制御ブロック
 */
typedef struct CsvItemCollection_t {
  //! @name Private
  //! @{

  CsvItem_t *list; //!< アイテムの配列
  size_t length;   //!< 現在の要素数
  size_t capacity; //!< 確保済みのキャパシティ

  //! @}
} CsvItemCollection_t;

/**
 * @brief CSVアイテムコレクションを初期化する。
 * @param[in,out] self CsvItemCollection_t 構造体へのポインタ
 */
extern void CsvItemCollection_Init(CsvItemCollection_t *self);

/**
 * @brief 内部バッファの容量を変更する。
 * @param[in,out] self        CsvItemCollection_t 構造体へのポインタ
 * @param[in]     newCapacity 新しい容量（要素数）
 */
extern void CsvItemCollection_Resize(CsvItemCollection_t *self,
                                     size_t newCapacity);

/**
 * @brief コレクションの所有権を別のオブジェクトに移動する。
 * @param[in,out] self     移動元の CsvItemCollection_t 構造体へのポインタ
 * @param[in,out] newOwner 移動先の CsvItemCollection_t 構造体へのポインタ
 */
extern void CsvItemCollection_MoveOwner(CsvItemCollection_t *self,
                                        CsvItemCollection_t *newOwner);

/**
 * @brief アイテムの所有権を移動し、コレクションの末尾に追加する。
 * @param[in,out] self CsvItemCollection_t 構造体へのポインタ
 * @param[in,out] item 追加する CsvItem_t 構造体へのポインタ（所有権が移動する）
 */
extern void CsvItemCollection_MoveAndAdd(CsvItemCollection_t *self,
                                         CsvItem_t *item);

/**
 * @brief CSVアイテムコレクションを破棄し、リソースを解放する。
 * @param[in,out] self CsvItemCollection_t 構造体へのポインタ
 */
extern void CsvItemCollection_Destroy(CsvItemCollection_t *self);

/**
 * @brief コレクション内の全要素をクリアする（バッファは解放しない）。
 * @param[in,out] self CsvItemCollection_t 構造体へのポインタ
 */
extern void CsvItemCollection_Clear(CsvItemCollection_t *self);

/**
 * @brief CSVアイテムコレクションの内容を標準出力に表示する。
 * @param[in] self CsvItemCollection_t 構造体へのポインタ
 */
extern void CsvItemCollection_Print(const CsvItemCollection_t *self);

/**
 * @brief 2つのCSVアイテムコレクションが等しいか判定する。
 * @param[in] self  比較元の CsvItemCollection_t 構造体へのポインタ
 * @param[in] items 比較対象の CsvItemCollection_t 構造体へのポインタ
 * @return 等しい場合は true、それ以外は false
 */
extern bool CsvItemCollection_Equals(const CsvItemCollection_t *self,
                                     const CsvItemCollection_t *items);

#ifdef __cplusplus
}
#endif
