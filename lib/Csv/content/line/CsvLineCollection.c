/**
 * @file CsvLineCollection.c
 * @brief 行データのコレクション
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/content/line/CsvLineCollection.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @def
 * キャパシティーの初期値
 */
#define INITIAL_CAPACITY (32)

/**
 * @brief リサイズが必要かどうかを判定する。
 * @param[in] self インスタンス
 * @return リサイズが必要な場合はtrue、それ以外はfalse
 */
static inline bool NeedResize(CsvLineCollection_t *self) {
  return self->length >= self->capacity;
}

/**
 * @brief 行データコレクションを初期化する。
 *
 * @param[in,out] self 対象のオブジェクト
 */
void CsvLineCollection_Init(CsvLineCollection_t *self) {
  self->capacity = INITIAL_CAPACITY;
  self->list = calloc(self->capacity, sizeof(CsvLine_t));
  if (!self->list) {
    self->capacity = 0;
    return;
  }
  self->length = 0;
}

/**
 * @brief 内部バッファのサイズを変更する。
 *
 * @param[in,out] self        対象のオブジェクト
 * @param[in]     newCapacity 新しいキャパシティー
 */
void CsvLineCollection_Resize(CsvLineCollection_t *self, size_t newCapacity) {
  CsvLine_t *newList = realloc(self->list, newCapacity * sizeof(CsvLine_t));
  if (!newList) {
    return; // 失敗時は既存バッファを維持
  }
  self->list = newList;
  self->capacity = newCapacity;
}

/**
 * @brief 行データをコレクションに追加する。
 *
 * 引数で渡された行データの所有権はコレクションに移動する。
 *
 * @param[in,out] self     対象のオブジェクト
 * @param[in,out] moveLine 追加する行データ（所有権が移動する）
 */
void CsvLineCollection_MoveAndAdd(CsvLineCollection_t *self, CsvLine_t *line) {
  if (NeedResize(self)) {
    CsvLineCollection_Resize(self, self->capacity * 2);
  }
  CsvLine_MoveOwner(line, &self->list[self->length++]);
}

/**
 * @brief オブジェクトを破棄する。
 *
 * 確保されたリソースを解放する。
 *
 * @param[in,out] self 対象のオブジェクト
 */
void CsvLineCollection_Destroy(CsvLineCollection_t *self) {
  for (size_t i = 0; i < self->length; i++) {
    CsvLine_t *line = &self->list[i];
    CsvLine_Destroy(line);
  }
  free(self->list);
  self->list = NULL;
  self->length = 0;
  self->capacity = 0;
}

/**
 * @brief コレクションの内容を標準出力に表示する。
 *
 * デバッグ目的で使用する。
 *
 * @param[in] self 対象のオブジェクト
 */
void CsvLineCollection_Print(const CsvLineCollection_t *self) {
  for (size_t i = 0; i < self->length; i++) {
    const CsvLine_t *line = &self->list[i];
    CsvLine_Print(line);
    printf("\n");
  }
}

/**
 * @brief 2つのコレクションが等価であるかを確認する。
 *
 * @param[in] self  比較元のオブジェクト
 * @param[in] lines 比較対象のオブジェクト
 * @return 等しい場合は true、そうでない場合は false
 */
bool CsvLineCollection_Equals(const CsvLineCollection_t *self,
                              const CsvLineCollection_t *lines) {
  if (self->length != lines->length) {
    return false;
  }
  for (size_t i = 0; i < self->length; i++) {
    const CsvLine_t *line1 = &self->list[i];
    const CsvLine_t *line2 = &lines->list[i];
    if (!CsvLine_Equals(line1, line2)) {
      return false;
    }
  }
  return true;
}
