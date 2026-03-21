/**
 * @file CsvLine.h
 * @brief csvライン
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../item/CsvItemCollection.h"
#include <stdbool.h>

/**
 * @brief 制御ブロック
 */
typedef struct CsvLine_t {
  //! @name Private
  //! @{

  CsvItemCollection_t items;  //!< アイテムリスト

  //! @}
} CsvLine_t;

/**
 * @brief CsvLine を初期化する。
 * @param[in,out] self CsvLine_t 構造体へのポインタ
 */
extern void CsvLine_Init(CsvLine_t *self);

/**
 * @brief 所有権を新しいオブジェクトに移動する。
 * @param[in,out] self     移動元の CsvLine_t 構造体へのポインタ
 * @param[in,out] newOwner 移動先の CsvLine_t 構造体へのポインタ
 */
extern void CsvLine_MoveOwner(CsvLine_t *self, CsvLine_t *newOwner);

/**
 * @brief アイテムの所有権を移動し、ラインの末尾に追加する。
 * @param[in,out] self CsvLine_t 構造体へのポインタ
 * @param[in,out] item 追加する CsvItem_t 構造体へのポインタ（所有権が移動する）
 */
extern void CsvLine_MoveBackItem(CsvLine_t *self, CsvItem_t *item);

/**
 * @brief CsvLine を破棄し、リソースを解放する。
 * @param[in,out] self CsvLine_t 構造体へのポインタ
 */
extern void CsvLine_Destroy(CsvLine_t *self);

/**
 * @brief CsvLine の内容を標準出力に表示する。
 * @param[in] self CsvLine_t 構造体へのポインタ
 */
extern void CsvLine_Print(const CsvLine_t *self);

/**
 * @brief 指定された CsvLine と等しいか判定する。
 * @param[in] self 比較元の CsvLine_t 構造体へのポインタ
 * @param[in] line 比較対象の CsvLine_t 構造体へのポインタ
 * @return 等しい場合は true、そうでない場合は false
 */
extern bool CsvLine_Equals(const CsvLine_t *self, const CsvLine_t *line);

#ifdef __cplusplus
}
#endif
