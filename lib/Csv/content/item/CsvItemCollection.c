/**
 * @file CsvItemCollection.c
 * @brief アイテムコレクション
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/content/item/CsvItemCollection.h"
#include "Csv/content/item/CsvItem.h"
#include "utilities.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! @brief キャパシティの初期値
#define INITIAL_CAPACITY (32)

/**
 * @brief リサイズが必要かどうかを判定する。
 * @param[in] self インスタンス
 * @return リサイズが必要な場合はtrue、それ以外はfalse
 */
static inline bool NeedResize(CsvItemCollection_t *self) {
  return self->length >= self->capacity;
}

/**
 * @brief CSVアイテムコレクションを初期化する。
 * @param[in,out] self CsvItemCollection_t構造体へのポインタ
 */
void CsvItemCollection_Init(CsvItemCollection_t *self) {
  CLEAR(self);
  self->capacity = INITIAL_CAPACITY;
  self->list = calloc(self->capacity, sizeof(CsvItem_t));
  if (!self->list) {
    self->capacity = 0;
    return;
  }
  self->length = 0;
}

/**
 * @brief CSVアイテムコレクションの容量を変更する。
 * @param[in,out] self        CsvItemCollection_t構造体へのポインタ
 * @param[in]     newCapacity 新しい容量
 */
void CsvItemCollection_Resize(CsvItemCollection_t *self, size_t newCapacity) {
  CsvItem_t *newList = realloc(self->list, newCapacity * sizeof(CsvItem_t));
  if (!newList) {
    return; // 失敗時は既存バッファを維持
  }
  self->list = newList;
  self->capacity = newCapacity;
}

/**
 * @brief CSVアイテムコレクションの所有権を移動する。
 * @param[in,out] self     移動元のCsvItemCollection_t構造体へのポインタ
 * @param[in,out] newOwner 移動先のCsvItemCollection_t構造体へのポインタ
 */
void CsvItemCollection_MoveOwner(CsvItemCollection_t *self,
                                 CsvItemCollection_t *newOwner) {
  *newOwner = *self;
  CLEAR(self);
}

/**
 * @brief アイテムの所有権を移動し、コレクションに追加する。
 * @param[in,out] self CsvItemCollection_t構造体へのポインタ
 * @param[in,out] item 追加するCsvItem_t構造体へのポインタ
 */
void CsvItemCollection_MoveAndAdd(CsvItemCollection_t *self, CsvItem_t *item) {
  if (NeedResize(self)) {
    CsvItemCollection_Resize(self, self->capacity * 2);
  }
  CsvItem_MoveOwner(item, &self->list[self->length++]);
}

/**
 * @brief CSVアイテムコレクションを破棄する。
 * @param[in,out] self CsvItemCollection_t構造体へのポインタ
 */
void CsvItemCollection_Destroy(CsvItemCollection_t *self) {
  for (size_t i = 0; i < self->length; i++) {
    CsvItem_t *item = &self->list[i];
    CsvItem_Destroy(item);
  }
  free(self->list);
  self->list = NULL;
  self->length = 0;
  self->capacity = 0;
}

/**
 * @brief CSVアイテムコレクションの要素をクリアする。
 * @param[in,out] self CsvItemCollection_t構造体へのポインタ
 */
void CsvItemCollection_Clear(CsvItemCollection_t *self) {
  memset(self->list, 0, sizeof(*self->list) * self->length);
  self->length = 0;
}

/**
 * @brief CSVアイテムコレクションの内容を表示する。
 * @param[in] self CsvItemCollection_t構造体へのポインタ
 */
void CsvItemCollection_Print(const CsvItemCollection_t *self) {
  for (size_t i = 0; i < self->length; i++) {
    const CsvItem_t *item = &self->list[i];
    CsvItem_Print(item);
    printf(", ");
  }
}

/**
 * @brief 2つのCSVアイテムコレクションが等しいかどうかを判定する。
 * @param[in] self  比較元のCsvItemCollection_t構造体へのポインタ
 * @param[in] items 比較対象のCsvItemCollection_t構造体へのポインタ
 * @return 等しい場合はtrue、それ以外はfalse
 */
bool CsvItemCollection_Equals(const CsvItemCollection_t *self,
                              const CsvItemCollection_t *items) {
  if (self->length != items->length) {
    return false;
  }

  for (size_t i = 0; i < self->length; i++) {
    const CsvItem_t *item1 = &self->list[i];
    const CsvItem_t *item2 = &items->list[i];
    if (!CsvItem_Equals(item1, item2)) {
      return false;
    }
  }
  return true;
}
