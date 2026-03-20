/**
 * @file CsvItem.h
 * @brief csvアイテム
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

/**
 * @brief 制御ブロック
 */
typedef struct CsvItem_t {
  //! @name Private
  //! @{

  char *text; //!< フィールドの文字列データ（ファイルバッファへのポインタ）

  //! @}
} CsvItem_t;

/**
 * @brief CSVアイテムを初期化する。
 * @param[in,out] self CsvItem_t 構造体へのポインタ
 */
extern void CsvItem_Init(CsvItem_t *self);

/**
 * @brief CSVアイテムに文字列ポインタを設定する。
 * @param[in,out] self  CsvItem_t 構造体へのポインタ
 * @param[in]     value 設定する文字列へのポインタ
 */
extern void CsvItem_Set(CsvItem_t *self, char *value);

/**
 * @brief CSVアイテムの所有権を別のオブジェクトに移動する。
 * @param[in,out] self     移動元の CsvItem_t 構造体へのポインタ
 * @param[in,out] newOwner 移動先の CsvItem_t 構造体へのポインタ
 */
extern void CsvItem_MoveOwner(CsvItem_t *self, CsvItem_t *newOwner);

/**
 * @brief CSVアイテムを破棄する。
 * @param[in,out] self CsvItem_t 構造体へのポインタ
 */
extern void CsvItem_Destroy(CsvItem_t *self);

/**
 * @brief CSVアイテムの内容を標準出力に表示する。
 * @param[in] self CsvItem_t 構造体へのポインタ
 */
extern void CsvItem_Print(const CsvItem_t *self);

/**
 * @brief 2つのCSVアイテムが等しいか判定する。
 * @param[in] self 比較元の CsvItem_t 構造体へのポインタ
 * @param[in] item 比較対象の CsvItem_t 構造体へのポインタ
 * @return 等しい場合は true、それ以外は false
 */
extern bool CsvItem_Equals(const CsvItem_t *self, const CsvItem_t *item);

#ifdef __cplusplus
}
#endif
