/**
 * @file Dictionary.c
 * @brief 辞書型
 * @author atohs
 * @date 2024/07/12
 */
#include "ExtendedTypes/Dictionary.h"
#include "utilities.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief メモリープールからバッファを割り当てる
 * @param[in,out] self インスタンス
 */
static void AssignMemory(Dictionary_t *self) {
  uint8_t *current = self->memoryPool;
  self->elements.list = (DictionaryElement_t *)current;

  current += (self->elements.capacity * sizeof(DictionaryElement_t));
  for (size_t i = 0; i < self->elements.capacity; i++) {
    DictionaryElement_t *e = &self->elements.list[i];
    e->key = (char *)current;
    current += self->maxKeySize;
    e->object.buffer = current;
    current += self->maxObjectSize;
  }
}

/**
 * @brief 辞書型を初期化する
 * @param[in,out] self          インスタンス
 * @param[in]     capacity      最大容量
 * @param[in]     maxKeySize    キーの最大長
 * @param[in]     maxObjectSize 要素のサイズ
 */
void Dictionary_Init(Dictionary_t *self,
                     size_t capacity,
                     size_t maxKeySize,
                     size_t maxObjectSize) {
  if (UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);
  self->elements.capacity = capacity;
  self->maxKeySize = maxKeySize;
  self->maxObjectSize = maxObjectSize;

  if (hcreate_r(self->elements.capacity, &self->hashTable) == 0) {
    CLEAR(self);
    return;
  }

  size_t allocateSize = self->elements.capacity * sizeof(DictionaryElement_t) +
                        self->elements.capacity * self->maxKeySize +
                        self->elements.capacity * self->maxObjectSize;
  self->memoryPool = malloc(allocateSize);
  if (UNLIKELY(!self->memoryPool)) {
    hdestroy_r(&self->hashTable);
    CLEAR(self);
    return;
  }
  AssignMemory(self);
}

/**
 * @brief 辞書に要素を追加する
 * @param[in,out] self       インスタンス
 * @param[in]     key        キー
 * @param[in]     object     要素
 * @param[in]     objectSize サイズ
 * @return 追加結果ステータス
 */
DictionaryReturn Dictionary_Add(Dictionary_t *self,
                                const char *key,
                                void *object,
                                size_t objectSize) {
  if (UNLIKELY(!self || !key || !object || !self->memoryPool)) {
    return DICTIONARY_INVALID_ARG;
  }

  size_t keySize = strlen(key) + 1;  // NOTE: null文字までコピーするので+1

  if ((keySize > self->maxKeySize) || (objectSize > self->maxObjectSize)) {
    return DICTIONARY_INVALID_ARG;
  }
  if (self->elements.count >= self->elements.capacity) {
    return DICTIONARY_FULL;
  }

  DictionaryElement_t *element = &self->elements.list[self->elements.count];

  memcpy(element->key, key, keySize);
  ENTRY e = {.key = element->key, .data = element};
  ENTRY *ep = NULL;
  if (hsearch_r(e, ENTER, &ep, &self->hashTable) == 0) {
    return DICTIONARY_FULL;
  }
  // hsearch_r(ENTER) は既存キーがあると上書きせず既存エントリを返す
  if (ep->data != element) {
    return DICTIONARY_DUPLICATED;
  }

  memcpy(element->object.buffer, object, objectSize);
  element->object.size = objectSize;
  self->elements.count++;
  return DICTIONARY_ADDED;
}

/**
 * @brief 指定されたキーの要素を取得する
 * @param[in] self インスタンス
 * @param[in] key  キー
 * @return 要素へのポインタ。見つからない場合はNULL
 */
const DictionaryObject_t *Dictionary_Find(Dictionary_t *self, const char *key) {
  if (UNLIKELY(!self || !key || !self->memoryPool)) {
    return NULL;
  }
  if ((strlen(key) + 1) > self->maxKeySize) {
    return NULL;
  }
  ENTRY e = {.key = (char *)key};
  ENTRY *foundObject = NULL;
  if (hsearch_r(e, FIND, &foundObject, &self->hashTable) == 0) {
    return NULL;
  }
  if (!foundObject)
    return NULL;
  return &((DictionaryElement_t *)foundObject->data)->object;
}

/**
 * @brief インスタンスを破棄する
 * @param[in,out] self インスタンス
 */
void Dictionary_Destroy(Dictionary_t *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  if (self->memoryPool) {
    hdestroy_r(&self->hashTable);
    free(self->memoryPool);
  }
  CLEAR(self);
}
