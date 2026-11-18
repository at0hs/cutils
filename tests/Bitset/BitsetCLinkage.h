#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Bitset の inline 関数を C から一通り呼び出し、結果を数値で返す (期待値 113)
uint64_t BitsetCLinkage_Run(void);

#ifdef __cplusplus
}
#endif
