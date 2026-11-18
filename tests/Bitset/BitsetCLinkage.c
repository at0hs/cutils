/**
 * @file BitsetCLinkage.c
 * @brief Bitset のヘッダ inline 関数を C (最適化なし) から呼び出せることを確認する
 * @details
 * C17 ではヘッダの inline 定義だけでは外部シンボルが生成されないため、
 * ライブラリ側に外部定義が無いと -O0 でリンクエラーになる。
 * このファイルは C として -O0 でコンパイルされ、リンクできること自体がテストとなる。
 */
#include "BitsetCLinkage.h"
#include "Bitset/Bitset.h"

uint64_t BitsetCLinkage_Run(void) {
  Bitset_t bs;
  Bitset_Init(&bs, 70);
  Bitset_Clear(&bs);
  Bitset_Set(&bs, 0);
  Bitset_Set(&bs, 69);
  Bitset_Reset(&bs, 0);
  Bitset_Or(&bs, 0x6);
  Bitset_And(&bs, 0x2);
  Bitset_ExclusiveOr(&bs, 0x1);
  Bitset_Flip(&bs);
  Bitset_Flip(&bs);
  uint64_t result = Bitset_Count(&bs);  // bit0, bit1, bit69 => 3
  result += Bitset_Test(&bs, 69) ? 10 : 0;
  result += Bitset_Any(&bs) ? 100 : 0;
  result += Bitset_All(&bs) ? 1000 : 0;
  result += Bitset_None(&bs) ? 10000 : 0;
  Bitset_Destroy(&bs);
  return result;  // 期待値 113
}
