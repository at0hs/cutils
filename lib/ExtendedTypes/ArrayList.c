/**
 * @file ArrayList.c
 * @brief リスト
 * @author atohs
 * @date 2024/08/06
 */
#include "ExtendedTypes/ArrayList.h"
#include "ExtendedTypes/MemoryAllocator.h"
#include "utilities.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//! @brief リストの初期容量
#define INITIAL_CAPACITY (64)

/**
 * @brief 指定されたインデックスのノードを取得する
 * @param[in] self リストインスタンス
 * @param[in] index インデックス
 * @return ノードへのポインタ
 */
static inline ArrayListNode *GetNode(const ArrayList *self, uint64_t index) {
  return &self->list[index];
}

/**
 * @brief 指定された位置にオブジェクトを挿入する（ディープコピー）
 * @param[in,out] self   リストインスタンス
 * @param[in]     index  挿入位置
 * @param[in]     object 挿入するオブジェクト
 */
static void Insert(ArrayList *self, uint64_t index, const void *object) {
  ArrayListNode *node = GetNode(self, index);
  node->object = self->allocator.allocate(self->objectSize);
  if (UNLIKELY(!node->object)) {
    return;
  }
  memcpy(node->object, object, self->objectSize);
  node->hadEntity = true;
  self->length++;
}

/**
 * @brief 指定された位置にオブジェクトの参照を挿入する
 * @param[in,out] self   リストインスタンス
 * @param[in]     index  挿入位置
 * @param[in]     object 挿入するオブジェクトへのポインタ
 */
static void InsertReference(ArrayList *self, uint64_t index, void *object) {
  ArrayListNode *node = GetNode(self, index);
  node->object = object;
  node->hadEntity = false;
  self->length++;
}

/**
 * @brief ノードを指定オフセット分シフトする
 * @param[in,out] self   リストインスタンス
 * @param[in]     index  起点となるインデックス
 * @param[in]     offset シフトするオフセット量
 * @return 移動先のメモリアドレス
 */
static inline void *Shift(ArrayList *self, uint64_t index, uint64_t offset) {
  return memmove(GetNode(self, index),
                 GetNode(self, index + offset),
                 (self->length - index - offset) * sizeof(ArrayListNode));
}

/**
 * @brief ノードをクリアする
 * 実体を持っている場合はアロケータを使用してメモリを解放する
 * @param[in]     self リストインスタンス
 * @param[in,out] node クリアするノード
 */
static inline void ClearNode(ArrayList *self, ArrayListNode *node) {
  if (node->hadEntity) {
    self->allocator.dispose(node->object);
    node->hadEntity = false;
  }
  node->object = NULL;
}

/**
 * @brief 指定されたインデックスの要素を削除し、詰める
 * @param[in,out] self  リストインスタンス
 * @param[in]     index 削除するインデックス
 */
static void RemoveAt(ArrayList *self, uint64_t index) {
  ArrayListNode *node = GetNode(self, index);
  ClearNode(self, node);
  Shift(self, index, 1);
  self->length--;
}

/**
 * @brief ArrayListを初期化する
 * @param[out] self       リストインスタンス
 * @param[in]  allocator  メモリアロケータ（NULLの場合はデフォルトを使用）
 * @param[in]  objectSize 格納するオブジェクトのサイズ
 */
void ArrayList_Init(ArrayList *self,
                    const MemoryAllocator_t *allocator,
                    size_t objectSize) {
  if (UNLIKELY(!self || objectSize == 0)) {
    return;
  }
  CLEAR(self);
  self->capacity = INITIAL_CAPACITY;
  self->objectSize = objectSize;
  self->list = calloc(self->capacity, sizeof(ArrayListNode));
  if (UNLIKELY(!self->list)) {
    return;
  }
  if (allocator) {
    self->allocator = *allocator;
  } else {
    self->allocator = *MemoryAllocator_GetDefault();
  }
}

/**
 * @brief ArrayListを破棄する
 * 確保されたすべてのリソースを解放する
 * @param[in,out] self リストインスタンス
 */
void ArrayList_Destroy(ArrayList *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  ArrayList_Clear(self);
  free(self->list);
  CLEAR(self);
}

/**
 * @brief リストが空かどうかを判定する
 * @param[in] self リストインスタンス
 * @return 空の場合は true、それ以外は false
 */
bool ArrayList_IsEmpty(const ArrayList *self) {
  if (UNLIKELY(!self)) {
    return true;
  }
  return self->length == 0;
}

/**
 * @brief リストの容量を変更する
 * @param[in,out] self    リストインスタンス
 * @param[in]     newSize 新しい容量
 */
void ArrayList_Resize(ArrayList *self, size_t newSize) {
  if (UNLIKELY(!self || (newSize == 0))) {
    return;
  }
  ArrayListNode *tmp = realloc(self->list, newSize * sizeof(ArrayListNode));
  if (UNLIKELY(!tmp)) {
    return;
  }
  self->list = tmp;
  self->capacity = newSize;
}

/**
 * @brief 格納されている要素数を取得する
 * @param[in] self リストインスタンス
 * @return 要素数
 */
size_t ArrayList_Length(const ArrayList *self) {
  if (UNLIKELY(!self)) {
    return 0;
  }
  return self->length;
}

/**
 * @brief 現在の容量を取得する
 * @param[in] self リストインスタンス
 * @return 容量
 */
size_t ArrayList_Capacity(const ArrayList *self) {
  if (UNLIKELY(!self)) {
    return 0;
  }
  return self->capacity;
}

/**
 * @brief 指定されたインデックスの要素を取得する
 * @param[in] self  リストインスタンス
 * @param[in] index インデックス
 * @return 要素へのポインタ。範囲外の場合は NULL
 */
const void *ArrayList_At(ArrayList *self, uint64_t index) {
  if (UNLIKELY(!self)) {
    return NULL;
  }
  if (index >= self->length) {
    return NULL;
  }
  return self->list[index].object;
}

/**
 * @brief リストの末尾に要素を追加する（ディープコピー）
 * 必要に応じて容量を自動的に拡張する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 追加するオブジェクト
 */
void ArrayList_Add(ArrayList *self, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return;
  }
  if (self->length >= self->capacity) {
    ArrayList_Resize(self, self->capacity * 2);
  }
  Insert(self, self->length, object);
}

/**
 * @brief リストの末尾に要素の参照を追加する
 * 必要に応じて容量を自動的に拡張する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 追加するオブジェクトへのポインタ
 */
void ArrayList_AddReference(ArrayList *self, void *object) {
  if (UNLIKELY(!self || !object)) {
    return;
  }
  if (self->length >= self->capacity) {
    ArrayList_Resize(self, self->capacity * 2);
  }
  InsertReference(self, self->length, object);
}

/**
 * @brief 複数の要素をリストの末尾に追加する
 * @param[in,out] self    リストインスタンス
 * @param[in]     objects 追加するオブジェクトの配列
 * @param[in]     length  追加する要素数
 */
void ArrayList_AddRange(ArrayList *self, const void *objects, size_t length) {
  if (UNLIKELY(!self || !objects || (length == 0))) {
    return;
  }
  const uint8_t *current = objects;
  for (size_t i = 0; i < length; i++) {
    ArrayList_Add(self, current);
    current += self->objectSize;
  }
}

/**
 * @brief 別のリストを現在のリストの末尾に結合する
 * 追加されるリストの各要素の管理状態（コピーか参照か）を引き継ぐ
 * @param[in,out] self リストインスタンス
 * @param[in]     list 追加するリスト
 */
void ArrayList_Append(ArrayList *self, const ArrayList *list) {
  if (UNLIKELY(!self || !list)) {
    return;
  }
  if (self->objectSize != list->objectSize) {
    return;
  }
  if (self->capacity < (self->length + list->length)) {
    ArrayList_Resize(self, self->capacity + list->capacity);
  }
  // memcpy(GetNode(self, self->length), list->list, list->length);
  size_t oldLength = self->length;
  for (size_t i = self->length, j = 0; i < (oldLength + list->length);
       i++, j++) {
    ArrayListNode *otherNode = GetNode(list, j);
    if (otherNode->hadEntity) {
      Insert(self, i, otherNode->object);
    } else {
      InsertReference(self, i, otherNode->object);
    }
  }
}

/**
 * @brief 指定された位置に要素を挿入する
 * @param[in,out] self   リストインスタンス
 * @param[in]     index  挿入位置
 * @param[in]     object 挿入するオブジェクト
 */
void ArrayList_Insert(ArrayList *self, uint64_t index, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return;
  }
  if (index >= self->length) {
    return;
  }
  if (self->length >= self->capacity) {
    ArrayList_Resize(self, self->capacity * 2);
  }
  ArrayListNode *node = GetNode(self, index);
  memmove(GetNode(self, index + 1),
          node,
          (self->length - index) * sizeof(ArrayListNode));
  Insert(self, index, object);
}

/**
 * @brief 指定されたオブジェクトのインデックスを検索する
 * メモリ内容の比較(memcmp)により判定する
 * @param[in] self   リストインスタンス
 * @param[in] object 検索するオブジェクト
 * @return 見つかったインデックス。見つからない場合は -1
 */
ssize_t ArrayList_IndexOf(ArrayList *self, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return -1;
  }
  for (uint64_t i = 0; i < self->length; i++) {
    if (memcmp(GetNode(self, i)->object, object, self->objectSize) == 0) {
      return (ssize_t)i;
    }
  }
  return -1;
}

/**
 * @brief リストに特定のオブジェクトが含まれているか確認する
 * @param[in] self   リストインスタンス
 * @param[in] object 検索するオブジェクト
 * @return 含まれている場合は true、それ以外は false
 */
bool ArrayList_Contains(ArrayList *self, const void *object) {
  return ArrayList_IndexOf(self, object) >= 0 ? true : false;
}

/**
 * @brief 指定されたオブジェクトをリストから削除する
 * 最初に見つかった要素のみを削除する
 * @param[in,out] self   リストインスタンス
 * @param[in]     object 削除するオブジェクト
 * @return 削除に成功した場合は true、要素が見つからない場合は false
 */
bool ArrayList_Remove(ArrayList *self, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return false;
  }
  for (uint64_t i = 0; i < self->length; i++) {
    ArrayListNode *node = GetNode(self, i);
    if (memcmp(node->object, object, self->objectSize) == 0) {
      RemoveAt(self, i);
      return true;
    }
  }
  return false;
}

/**
 * @brief 指定されたインデックスの要素を削除する
 * @param[in,out] self  リストインスタンス
 * @param[in]     index 削除するインデックス
 * @return 削除に成功した場合は true、インデックスが範囲外の場合は false
 */
bool ArrayList_RemoveAt(ArrayList *self, uint64_t index) {
  if (UNLIKELY(!self)) {
    return false;
  }
  if (index >= self->length) {
    return false;
  }
  RemoveAt(self, index);
  return true;
}

/**
 * @brief リストのすべての要素を削除する
 * @param[in,out] self リストインスタンス
 */
void ArrayList_Clear(ArrayList *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  for (size_t i = 0; i < self->length; i++) {
    ClearNode(self, GetNode(self, i));
  }
  self->length = 0;
}

/**
 * @brief リストの各要素に対してコールバック関数を実行する
 * @param[in] self     リストインスタンス
 * @param[in] callback 各要素に対して実行される関数
 */
void ArrayList_ForEach(ArrayList *self,
                       void (*callback)(uint64_t index, const void *object)) {
  if (UNLIKELY(!self || !callback)) {
    return;
  }
  for (size_t i = 0; i < self->length; i++) {
    callback(i, GetNode(self, i)->object);
  }
}
