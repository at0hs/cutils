/**
 * @file CsvLine.c
 * @brief 行のデータ
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/content/line/CsvLine.h"
#include "utilities.h"
#include <stdbool.h>

/**
 * @brief CsvLineを初期化する。
 * @param[in,out] self 対象オブジェクト
 */
void CsvLine_Init(CsvLine_t *self) {
  CLEAR(self);
  CsvItemCollection_Init(&self->items);
}

/**
 * @brief 所有権を新しいオブジェクトに移動する。
 * @param[in,out] self     移動元のオブジェクト
 * @param[in,out] newOwner 移動先のオブジェクト
 */
void CsvLine_MoveOwner(CsvLine_t *self, CsvLine_t *newOwner) {
  *newOwner = *self;
  CLEAR(self);
}

/**
 * @brief アイテムをラインの末尾に追加（移動）する。
 * @param[in,out] self 対象オブジェクト
 * @param[in]     item 追加するアイテム
 */
bool CsvLine_MoveBackItem(CsvLine_t *self, CsvItem_t *item) {
  return CsvItemCollection_MoveAndAdd(&self->items, item);
}

/**
 * @brief CsvLineを破棄する。
 * @param[in,out] self 対象オブジェクト
 */
void CsvLine_Destroy(CsvLine_t *self) {
  CsvItemCollection_Destroy(&self->items);
  CLEAR(self);
}

/**
 * @brief CsvLineの内容を表示する。
 * @param[in] self 対象オブジェクト
 */
void CsvLine_Print(const CsvLine_t *self) {
  CsvItemCollection_Print(&self->items);
}

/**
 * @brief 指定されたCsvLineと等しいか判定する。
 * @param[in] self 対象オブジェクト
 * @param[in] line 比較対象のライン
 * @return 等しい場合はtrue、そうでない場合はfalse
 */
bool CsvLine_Equals(const CsvLine_t *self, const CsvLine_t *line) {
  return CsvItemCollection_Equals(&self->items, &line->items);
}
