/**
 * @file MemoryAllocator.h
 * @brief メモリアロケーター
 * @author atohs
 * @date 2024/08/05
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
/**
 * @brief 制御ブロック
 */
typedef struct MemoryAllocator_t {
  //! @name private
  //! @{
  void *(*allocate)(size_t size);
  void (*dispose)(void *ptr);
  //! @}
} MemoryAllocator_t;

/**
 * @brief メモリアロケーターを初期化する
 * @param[out] self     アロケーターインスタンス
 * @param[in]  allocate メモリ確保関数（NULLの場合は malloc を使用）
 * @param[in]  dispose  メモリ解放関数（NULLの場合は free を使用）
 */
extern void MemoryAllocator_Init(MemoryAllocator_t *self,
                                 void *(*allocate)(size_t),
                                 void (*dispose)(void *));

/**
 * @brief デフォルトのメモリアロケーター（malloc/free）を取得する
 * @return デフォルトアロケーターへのポインタ
 */
extern const MemoryAllocator_t *MemoryAllocator_GetDefault();

#ifdef __cplusplus
}
#endif
