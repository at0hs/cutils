/**
 * @file utilities.h
 * @brief 汎用ユーティリティマクロおよび時刻取得関数
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "File.h"
#include <time.h>

/**
 * @brief 構造体をゼロクリアする
 * @param objectPointer クリア対象オブジェクトへのポインタ
 * @note `memset` を用いて `*objectPointer` のサイズ分だけゼロ初期化する
 */
#define CLEAR(objectPointer) memset(objectPointer, 0, sizeof(*objectPointer))

/**
 * @brief 条件式が真になりやすいことをコンパイラに伝えるヒント
 * @param condition 評価する条件式
 * @return `condition` を bool に変換した値
 */
#define LIKELY(condition) __glibc_likely(!!(condition))

/**
 * @brief 条件式が偽になりやすいことをコンパイラに伝えるヒント
 * @param condition 評価する条件式
 * @return `condition` を bool に変換した値
 */
#define UNLIKELY(condition) __glibc_unlikely(!!(condition))

/**
 * @brief リアルタイムクロック (CLOCK_REALTIME) の現在時刻をミリ秒で返す
 * @return エポックからの経過時間 (ミリ秒)
 */
time_t GetRealTime();

/**
 * @brief モノトニッククロック (CLOCK_MONOTONIC_RAW) の現在時刻をミリ秒で返す
 * @return モノトニック起点からの経過時間 (ミリ秒)
 * @note システム時刻の変更に影響されないため、経過時間の計測に適している
 */
time_t GetMonotonicTime();

#ifdef __cplusplus
}
#endif
