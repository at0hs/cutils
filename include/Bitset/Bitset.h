/**
 * @file Bitset.h
 * @brief ビット集合
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief 制御ブロック
 */
typedef struct Bitset_t {
  //! @name Private
  //! @{

  //! ビット集合
  uint64_t *words;
  //! キャパシティー
  uint64_t capacity;
  //! ビット数
  uint64_t numBits;
  //! 文字列バッファ
  char *string;

  //! @}
} Bitset_t;

/**
 * @brief ビット集合を初期化します。
 * @param[out] self     Bitset_t構造体へのポインタ
 * @param[in]  numBits  管理するビット数
 */
void Bitset_Init(Bitset_t *self, uint64_t numBits);

/**
 * @brief ビット集合を破棄します。
 * @param[in,out] self  Bitset_t構造体へのポインタ
 */
void Bitset_Destroy(Bitset_t *self);

/**
 * @brief ビット集合を右にシフトします。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     shift  シフト量
 */
void Bitset_RightShift(Bitset_t *self, uint64_t shift);

/**
 * @brief ビット集合を左にシフトします。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     shift  シフト量
 */
void Bitset_LeftShift(Bitset_t *self, uint64_t shift);

/**
 * @brief ビット集合の文字列表現を取得します。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return ビット集合を表す文字列（"0", "1" の配列）
 */
char *Bitset_ToString(Bitset_t *self);

/** @cond PRIVATE */

/** @brief 1ワードのバイト数（= sizeof(uint64_t)）*/
#define __BS_WORD_BYTE ((uint8_t)sizeof(*((Bitset_t *)NULL)->words))
/** @brief 1ワードのビット数（= 64）*/
#define __BS_WORD_BIT ((uint64_t)(__BS_WORD_BYTE * 8))
/**
 * @brief ビット位置をワード配列のインデックスに変換します。
 * @param v ビット位置
 */
#define __BS_TO_WORD(v) (v / __BS_WORD_BIT)
/**
 * @brief ビット位置をワード内のビットオフセットに変換します。
 * @param v ビット位置
 */
#define __BS_TO_BIT(v) (v % __BS_WORD_BIT)
/**
 * @brief 条件が偽になりやすいことをコンパイラに伝えます。
 * @param cond 評価する条件式
 */
#define __BS_UNLIKELY(cond) (__glibc_unlikely(!!(cond)))
/**
 * @brief 条件が真になりやすいことをコンパイラに伝えます。
 * @param cond 評価する条件式
 */
#define __BS_LIKELY(cond) (__glibc_likely(!!(cond)))

/** @endcond */

/**
 * @brief すべてのビットをゼロクリアします。
 * @param[in,out] self  Bitset_t構造体へのポインタ
 */
inline void Bitset_Clear(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  memset(self->words, 0, self->capacity * sizeof(*self->words));
}

/**
 * @brief 先頭ワードとの論理積をとります。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     value  演算する値（64bit）
 */
inline void Bitset_And(Bitset_t *self, uint64_t value) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  self->words[0] &= value;
}

/**
 * @brief 先頭ワードとの論理和をとります。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     value  演算する値（64bit）
 */
inline void Bitset_Or(Bitset_t *self, uint64_t value) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  self->words[0] |= value;
}

/**
 * @brief 先頭ワードとの排他的論理和をとります。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     value  演算する値（64bit）
 */
inline void Bitset_ExclusiveOr(Bitset_t *self, uint64_t value) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  self->words[0] ^= value;
}

/**
 * @brief 指定された位置のビットをセット（1に設定）します。
 * @param[in,out] self      Bitset_t構造体へのポインタ
 * @param[in]     position  ビット位置
 */
inline void Bitset_Set(Bitset_t *self, uint64_t position) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  if (position >= self->numBits) {
    return;
  }
  const uint64_t wordPos = __BS_TO_WORD(position);
  self->words[wordPos] |= (1UL << __BS_TO_BIT(position));
}

/**
 * @brief 指定された位置のビットをリセット（0に設定）します。
 * @param[in,out] self      Bitset_t構造体へのポインタ
 * @param[in]     position  ビット位置
 */
inline void Bitset_Reset(Bitset_t *self, uint64_t position) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  if (position >= self->numBits) {
    return;
  }
  const uint64_t wordPos = __BS_TO_WORD(position);
  self->words[wordPos] &= ~(1UL << __BS_TO_BIT(position));
}

/**
 * @brief すべてのビットを反転させます。
 * @param[in,out] self  Bitset_t構造体へのポインタ
 */
inline void Bitset_Flip(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  for (uint64_t i = 0; i < self->capacity; i++) {
    self->words[i] = ~self->words[i];
  }
  const uint64_t remainBits = __BS_TO_BIT(self->numBits);
  if (remainBits != 0) {
    self->words[self->capacity - 1] &= (1UL << remainBits) - 1;
  }
}

/**
 * @brief セットされている（1になっている）ビット数をカウントします。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return セットされているビット数
 */
inline uint64_t Bitset_Count(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return 0;
  }
  uint64_t counter = 0;
  for (uint64_t i = 0; i < self->capacity; i++) {
    counter += __builtin_popcountl(self->words[i]);
  }
  return counter;
}

/**
 * @brief 指定された位置のビットがセットされているか判定します。
 * @param[in] self      Bitset_t構造体へのポインタ
 * @param[in] position  ビット位置
 * @return ビットがセットされていればtrue、そうでなければfalse
 */
inline bool Bitset_Test(Bitset_t *self, uint64_t position) {
  if (__BS_UNLIKELY(!self)) {
    return false;
  }
  if (position >= self->numBits) {
    return false;
  }
  const uint64_t wordPos = __BS_TO_WORD(position);
  return self->words[wordPos] & (1UL << __BS_TO_BIT(position));
}

/**
 * @brief いずれかのビットがセットされているか判定します。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return 1つ以上のビットがセットされていればtrue、そうでなければfalse
 */
inline bool Bitset_Any(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return false;
  }

  for (uint64_t i = 0; i < self->capacity; i++) {
    if (self->words[i]) {
      return true;
    }
  }
  return false;
}

/**
 * @brief すべてのビットがセットされているか判定します。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return すべてのビットがセットされていればtrue、そうでなければfalse
 */
inline bool Bitset_All(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return false;
  }
  const uint64_t remainBits = __BS_TO_BIT(self->numBits);
  const uint64_t fullWords = remainBits ? self->capacity - 1 : self->capacity;
  for (uint64_t i = 0; i < fullWords; i++) {
    if (self->words[i] != UINT64_MAX) {
      return false;
    }
  }
  if (remainBits != 0) {
    const uint64_t mask = (1UL << remainBits) - 1;
    if (self->words[self->capacity - 1] != mask) {
      return false;
    }
  }
  return true;
}

/**
 * @brief すべてのビットが0（セットされていない）か判定します。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return すべてのビットが0であればtrue、そうでなければfalse
 */
inline bool Bitset_None(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return false;
  }
  return !Bitset_Any(self);
}

#ifdef __cplusplus
}
#endif
