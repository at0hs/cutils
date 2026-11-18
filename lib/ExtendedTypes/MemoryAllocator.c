/**
 * @file MemoryAllocator.c
 * @brief メモリアロケーター
 * @author atohs
 * @date 2024/08/06
 */
#include "ExtendedTypes/MemoryAllocator.h"
#include <stdlib.h>
#include <string.h>

static const MemoryAllocator_t defaultAllocator = {.allocate = malloc,
                                                   .dispose = free};

/**
 * @brief メモリアロケーターを初期化する
 * @param[out] self     アロケーターインスタンス
 * @param[in]  allocate メモリ確保関数（NULLの場合は malloc を使用）
 * @param[in]  dispose  メモリ解放関数（NULLの場合は free を使用）
 */
void MemoryAllocator_Init(MemoryAllocator_t *self,
                          void *(*allocate)(size_t),
                          void (*dispose)(void *)) {
  if (!self) {
    return;
  }
  memset(self, 0, sizeof(*self));
  self->allocate = allocate ? allocate : malloc;
  self->dispose = dispose ? dispose : free;
}

/**
 * @brief デフォルトのメモリアロケーター（malloc/free）を取得する
 * @return デフォルトアロケーターへのポインタ
 */
const MemoryAllocator_t *MemoryAllocator_GetDefault() {
  return &defaultAllocator;
}
