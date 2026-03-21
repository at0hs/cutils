/**
 * @file CsvItem.c
 * @brief csvのアイテム
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/content/item/CsvItem.h"
#include "utilities.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief CSVアイテムを初期化する。
 * @param[in,out] self CsvItem_t構造体へのポインタ
 */
void CsvItem_Init(CsvItem_t *self) {
  CLEAR(self);
}

/**
 * @brief CSVアイテムに値を設定する。
 * @param[in,out] self  CsvItem_t構造体へのポインタ
 * @param[in]     value 設定する文字列へのポインタ
 */
void CsvItem_Set(CsvItem_t *self, char *value) {
  self->text = value;
}

/**
 * @brief CSVアイテムの所有権を移動する。
 * @param[in,out] self     移動元のCsvItem_t構造体へのポインタ
 * @param[in,out] newOwner 移動先のCsvItem_t構造体へのポインタ
 */
void CsvItem_MoveOwner(CsvItem_t *self, CsvItem_t *newOwner) {
  *newOwner = *self;
  self->text = NULL;
}

/**
 * @brief CSVアイテムを破棄する。
 * @param[in,out] self CsvItem_t構造体へのポインタ
 */
void CsvItem_Destroy(CsvItem_t *self) {
  CLEAR(self);
}

/**
 * @brief CSVアイテムの内容を表示する。
 * @param[in] self CsvItem_t構造体へのポインタ
 */
void CsvItem_Print(const CsvItem_t *self) {
  printf("%s", self->text);
}

/**
 * @brief 2つのCSVアイテムが等しいか判定する。
 * @param[in] self 比較元のCsvItem_t構造体へのポインタ
 * @param[in] item 比較対象のCsvItem_t構造体へのポインタ
 * @return 等しい場合はtrue、それ以外はfalse
 */
bool CsvItem_Equals(const CsvItem_t *self, const CsvItem_t *item) {
  return strcmp(self->text, item->text) ? false : true;
}
