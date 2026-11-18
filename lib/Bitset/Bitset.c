/**
 * @file Bitset.c
 * @brief ビット集合
 * @author atohs
 * @date 2024/07/12
 */
#include "Bitset/Bitset.h"
#include "utilities.h"
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief メモリ領域を指定された値で埋めます。
 * @param p 埋めるメモリ領域へのポインタ
 * @param v 埋める値（バイト単位）
 * @param s 埋めるワード数
 */
#define fill(p, v, s) (memset((p), (v), (s) * __BS_WORD_BYTE))

/*
 * ヘッダで定義している inline 関数の外部定義をこの翻訳単位に生成する。
 * C17 では inline 定義だけでは外部シンボルが作られないため、これが無いと
 * C の利用側で (特に -O0 で) undefined reference になる。
 */
extern inline void Bitset_Clear(Bitset_t *self);
extern inline void Bitset_And(Bitset_t *self, uint64_t value);
extern inline void Bitset_Or(Bitset_t *self, uint64_t value);
extern inline void Bitset_ExclusiveOr(Bitset_t *self, uint64_t value);
extern inline void Bitset_Set(Bitset_t *self, uint64_t position);
extern inline void Bitset_Reset(Bitset_t *self, uint64_t position);
extern inline void Bitset_Flip(Bitset_t *self);
extern inline uint64_t Bitset_Count(Bitset_t *self);
extern inline bool Bitset_Test(Bitset_t *self, uint64_t position);
extern inline bool Bitset_Any(Bitset_t *self);
extern inline bool Bitset_All(Bitset_t *self);
extern inline bool Bitset_None(Bitset_t *self);

/**
 * @brief ビット集合を初期化します。
 * @param[out] self     Bitset_t構造体へのポインタ
 * @param[in]  numBits  管理するビット数
 */
void Bitset_Init(Bitset_t *self, uint64_t numBits) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);
  self->numBits = numBits;
  self->capacity =
      __BS_TO_BIT(numBits) ? __BS_TO_WORD(numBits) + 1 : __BS_TO_WORD(numBits);
  self->words = calloc(self->capacity, sizeof(uint64_t));
  if (!self->words) {
    CLEAR(self);
  }
}

/**
 * @brief ビット集合を破棄します。
 * @param[in,out] self  Bitset_t構造体へのポインタ
 */
void Bitset_Destroy(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  free(self->words);
  if (self->string)
    free(self->string);
  CLEAR(self);
}

/**
 * @brief ビット集合を右にシフトします。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     shift  シフト量
 */
void Bitset_RightShift(Bitset_t *self, uint64_t shift) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  if (shift == 0) {
    return;
  }
  const uint64_t shiftWords = __BS_TO_WORD(shift);
  if (shiftWords >= self->capacity) {
    fill(self->words, 0, self->capacity);
    return;
  }
  const uint64_t offset = __BS_TO_BIT(shift);
  const uint64_t limit = self->capacity - shiftWords - 1;

  if (offset == 0) {
    for (uint64_t i = 0; i <= limit; i++) {
      self->words[i] = self->words[i + shiftWords];
    }
  } else {
    const uint64_t subOffset = __BS_WORD_BIT - offset;
    for (uint64_t i = 0; i < limit; i++) {
      self->words[i] = ((self->words[i + shiftWords] >> offset) |
                        (self->words[i + shiftWords + 1] << subOffset));
    }
    self->words[limit] = self->words[limit + shiftWords] >> offset;
  }
  fill(&self->words[limit + 1], 0, self->capacity - limit - 1);
}

/**
 * @brief ビット集合を左にシフトします。
 * @param[in,out] self   Bitset_t構造体へのポインタ
 * @param[in]     shift  シフト量
 */
void Bitset_LeftShift(Bitset_t *self, uint64_t shift) {
  if (__BS_UNLIKELY(!self)) {
    return;
  }
  if (shift == 0) {
    return;
  }
  const uint64_t shiftWords = __BS_TO_WORD(shift);
  if (shiftWords >= self->capacity) {
    fill(self->words, 0, self->capacity);
    return;
  }
  const uint64_t offset = __BS_TO_BIT(shift);

  if (offset == 0) {
    for (uint64_t i = self->capacity - 1; i >= shiftWords; i--) {
      self->words[i] = self->words[i - shiftWords];
    }
  } else {
    const uint64_t subOffset = __BS_WORD_BIT - offset;
    for (uint64_t i = self->capacity - 1; i > shiftWords; i--) {
      self->words[i] = ((self->words[i - shiftWords] << offset) |
                        (self->words[i - shiftWords - 1] >> subOffset));
    }
    self->words[shiftWords] = self->words[0] << offset;
  }
  fill(self->words, 0, shiftWords);
  // numBits を超えて押し出されたビットを捨てる
  __BS_MASK_TAIL(self);
}

/**
 * @brief 指定された数値を2進数文字列に変換します。
 * @param[in]  v       変換する数値
 * @param[out] buffer  格納先のバッファ
 * @param[in]  length  出力するビット数
 */
static inline void BinaryToString(uint64_t v, char *buffer, uint8_t length) {
  if (length == 0) {
    return;
  }
  uint64_t offset = __BS_WORD_BIT - length;
  for (uint64_t mask = 0x8000000000000000 >> offset; mask; mask >>= 1) {
    *buffer++ = (v & mask) ? '1' : '0';
  }
}

/**
 * @brief ビット集合の文字列表現を取得します。
 * @param[in] self  Bitset_t構造体へのポインタ
 * @return ビット集合を表す文字列（"0", "1" の配列）
 */
char *Bitset_ToString(Bitset_t *self) {
  if (__BS_UNLIKELY(!self)) {
    return NULL;
  }
  if (!self->string) {
    self->string = calloc(self->numBits + 1, 1);
    if (__BS_UNLIKELY(!self->string)) {
      return NULL;
    }
  }
  int64_t numWords = __BS_TO_WORD(self->numBits);
  uint8_t remainBits = __BS_TO_BIT(self->numBits);
  char *str = self->string;
  if (remainBits != 0) {
    BinaryToString(self->words[numWords], str, remainBits);
    str += remainBits;
  }
  for (int64_t i = numWords - 1; i >= 0; i--) {
    BinaryToString(self->words[i], str, __BS_WORD_BIT);
    str += __BS_WORD_BIT;
  }
  *str = '\0';
  return self->string;
}
