/**
 * @file LinkedList.c
 * @brief 単方向リスト
 * @author atohs
 * @date 2024/08/05
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "ExtendedTypes/LinkedList.h"

/* --------------------------------------------------------------------------
 * プライベートヘルパー
 * -------------------------------------------------------------------------- */

/**
 * @brief 新しいノードを生成してオブジェクトをディープコピーする
 * @param[in] self   リストインスタンス
 * @param[in] object コピー元オブジェクト
 * @return 生成したノードへのポインタ
 */
static inline LinkedListNode_t *CreateNewNode(LinkedList_t *self,
                                              const void *object) {
  LinkedListNode_t *node = calloc(1, sizeof(LinkedListNode_t));
  node->object = self->allocator.allocate(self->objectSize);
  memcpy(node->object, object, self->objectSize);
  return node;
}

/**
 * @brief ノードのオブジェクトとノード自身を解放する
 * @param[in] self リストインスタンス
 * @param[in] node 解放するノード
 */
static void DestroyNode(LinkedList_t *self, LinkedListNode_t *node) {
  self->allocator.dispose(node->object);
  free(node);
}

/**
 * @brief 指定されたインデックスのノードを取得する
 * @param[in] self  リストインスタンス
 * @param[in] index インデックス（0 始まり）
 * @return 該当ノードへのポインタ。範囲外の場合は NULL
 */
static LinkedListNode_t *GetAt(LinkedList_t *self, uint32_t index) {
  if (index >= self->count) {
    return NULL;
  }
  uint32_t counter = 0;
  LinkedListNode_t *current = self->head;
  while (current && index != counter) {
    current = current->next;
    counter++;
  }
  return current;
}

/**
 * @brief 末尾ノードを取得する
 * @param[in] self リストインスタンス（空でないこと）
 * @return 末尾ノードへのポインタ
 */
static LinkedListNode_t *FindTail(LinkedList_t *self) {
  LinkedListNode_t *node = self->head;
  while (node->next) {
    node = node->next;
  }
  return node;
}

/* --------------------------------------------------------------------------
 * 公開 API
 * -------------------------------------------------------------------------- */

void LinkedList_Init(LinkedList_t *self,
                     const MemoryAllocator_t *allocator,
                     size_t objectSize) {
  if (UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);
  self->objectSize = objectSize;
  if (allocator) {
    self->allocator = *allocator;
  } else {
    self->allocator = *MemoryAllocator_GetDefault();
  }
}

bool LinkedList_IsEmpty(LinkedList_t *self) {
  if (UNLIKELY(!self)) {
    return true;
  }
  return self->count == 0;
}

size_t LinkedList_Count(LinkedList_t *self) {
  if (UNLIKELY(!self)) {
    return 0;
  }
  return self->count;
}

void LinkedList_Add(LinkedList_t *self, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return;
  }
  LinkedListNode_t *node = CreateNewNode(self, object);
  if (LinkedList_IsEmpty(self)) {
    self->head = node;
  } else {
    LinkedListNode_t *tail = FindTail(self);
    tail->next = node;
  }
  self->count++;
}

const void *LinkedList_At(LinkedList_t *self, uint32_t index) {
  if (UNLIKELY(!self)) {
    return NULL;
  }
  LinkedListNode_t *node = GetAt(self, index);
  return node ? node->object : NULL;
}

bool LinkedList_Contains(LinkedList_t *self, void *object) {
  if (UNLIKELY(!self || !object)) {
    return false;
  }
  const LinkedListNode_t *node;
  LinkedList_Foreach(self, node) {
    if (memcmp(object, node->object, self->objectSize) == 0) {
      return true;
    }
  }
  return false;
}

bool LinkedList_Remove(LinkedList_t *self, const void *object) {
  if (UNLIKELY(!self || !object)) {
    return false;
  }
  if (LinkedList_IsEmpty(self)) {
    return false;
  }

  LinkedListNode_t *current = self->head;
  if (memcmp(current->object, object, self->objectSize) == 0) {
    self->head = current->next;
    DestroyNode(self, current);
    self->count--;
    return true;
  }
  while (current->next) {
    if (memcmp(current->next->object, object, self->objectSize) == 0) {
      LinkedListNode_t *tmp = current->next;
      current->next = tmp->next;
      DestroyNode(self, tmp);
      self->count--;
      return true;
    }
    current = current->next;
  }
  return false;
}

bool LinkedList_RemoveAt(LinkedList_t *self, uint32_t index) {
  if (UNLIKELY(!self)) {
    return false;
  }
  if (index >= self->count) {
    return false;
  }

  if (index == 0) {
    LinkedListNode_t *tmp = self->head;
    self->head = tmp->next;
    DestroyNode(self, tmp);
    self->count--;
    return true;
  }
  LinkedListNode_t *node = GetAt(self, index - 1);
  LinkedListNode_t *tmp = node->next;
  node->next = tmp->next;
  DestroyNode(self, tmp);
  self->count--;
  return true;
}

void LinkedList_Clear(LinkedList_t *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  LinkedListNode_t *current = self->head;
  while (current) {
    LinkedListNode_t *next = current->next;
    DestroyNode(self, current);
    current = next;
  }
  self->head = NULL;
  self->count = 0;
}

void LinkedList_Destroy(LinkedList_t *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  LinkedList_Clear(self);
  CLEAR(self);
}
