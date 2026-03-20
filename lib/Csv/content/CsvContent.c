/**
 * @file CsvContent.c
 * @brief csvデータ
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/content/CsvContent.h"
#include "Csv/content/line/CsvLineCollection.h"
#include "utilities.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief CSVコンテンツを初期化する。
 * @param[in,out] self CsvContent_t構造体へのポインタ
 */
void CsvContent_Init(CsvContent_t *self) {
  CLEAR(self);
  CsvLineCollection_Init(&self->lines);
}

/**
 * @brief 行の所有権を移動し、コンテンツの末尾に追加する。
 * @param[in,out] self CsvContent_t構造体へのポインタ
 * @param[in,out] line 追加するCsvLine_t構造体へのポインタ
 */
void CsvContent_MoveBackLine(CsvContent_t *self, CsvLine_t *line) {
  CsvLineCollection_MoveAndAdd(&self->lines, line);
}

/**
 * @brief CSVコンテンツを破棄する。
 * @param[in,out] self CsvContent_t構造体へのポインタ
 */
void CsvContent_Destroy(CsvContent_t *self) {
  CsvLineCollection_Destroy(&self->lines);
  CLEAR(self);
}

/**
 * @brief CSVコンテンツの内容を表示する。
 * @param[in] self CsvContent_t構造体へのポインタ
 */
void CsvContent_Print(const CsvContent_t *self) {
  printf("Csv contents\n");
  CsvLineCollection_Print(&self->lines);
}

/**
 * @brief 2つのCSVコンテンツが等しいか判定する。
 * @param[in] self    比較元のCsvContent_t構造体へのポインタ
 * @param[in] content 比較対象のCsvContent_t構造体へのポインタ
 * @return 等しい場合はtrue、それ以外はfalse
 */
bool CsvContent_Equals(const CsvContent_t *self, const CsvContent_t *content) {
  return CsvLineCollection_Equals(&self->lines, &content->lines);
}
