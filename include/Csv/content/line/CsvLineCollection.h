/**
 * @file CsvLineCollection.h
 * @brief 行データのコレクション
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "CsvLine.h"
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief 制御ブロック
 */
typedef struct CsvLineCollection_t {
  //! @name Private
  //! @{

  CsvLine_t *list;  //!< 行データの配列
  size_t length;    //!< 現在の要素数
  size_t capacity;  //!< 確保済みのキャパシティ

  //! @}
} CsvLineCollection_t;

/**
 * @brief 行データコレクションを初期化する。
 * @param[in,out] self CsvLineCollection_t 構造体へのポインタ
 */
extern void CsvLineCollection_Init(CsvLineCollection_t *self);

/**
 * @brief 内部バッファの容量を変更する。
 * @param[in,out] self        CsvLineCollection_t 構造体へのポインタ
 * @param[in]     newCapacity 新しい容量（要素数）
 */
extern void CsvLineCollection_Resize(CsvLineCollection_t *self,
                                     size_t newCapacity);

/**
 * @brief 行データの所有権を移動し、コレクションの末尾に追加する。
 * @param[in,out] self     CsvLineCollection_t 構造体へのポインタ
 * @param[in,out] moveLine 追加する CsvLine_t
 * 構造体へのポインタ（所有権が移動する）
 * @return 追加できた場合は true。メモリ確保に失敗した場合は false（引数側は破棄される）
 */
extern bool CsvLineCollection_MoveAndAdd(CsvLineCollection_t *self,
                                         CsvLine_t *moveLine);

/**
 * @brief 行データコレクションを破棄し、リソースを解放する。
 * @param[in,out] self CsvLineCollection_t 構造体へのポインタ
 */
extern void CsvLineCollection_Destroy(CsvLineCollection_t *self);

/**
 * @brief コレクションの内容を標準出力に表示する。
 * @param[in] self CsvLineCollection_t 構造体へのポインタ
 */
extern void CsvLineCollection_Print(const CsvLineCollection_t *self);

/**
 * @brief 2つのコレクションが等価かどうかを判定する。
 * @param[in] self  比較元の CsvLineCollection_t 構造体へのポインタ
 * @param[in] lines 比較対象の CsvLineCollection_t 構造体へのポインタ
 * @return 等しい場合は true、そうでない場合は false
 */
extern bool CsvLineCollection_Equals(const CsvLineCollection_t *self,
                                     const CsvLineCollection_t *lines);

#ifdef __cplusplus
}
#endif
