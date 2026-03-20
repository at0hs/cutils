/**
 * @file utilities.c
 * @brief 汎用ユーティリティ関数の実装
 * @author atohs
 * @date 2024/07/12
 */
#include "utilities.h"
#include <time.h>

/**
 * @brief `struct timespec` をミリ秒単位の `time_t` に変換する
 * @param time 変換元の `timespec` 構造体へのポインタ
 * @return ミリ秒単位の時刻値
 */
static inline time_t TimespecToMilliseconds(const struct timespec *time) {
  return (time->tv_sec * 1000) + (time->tv_nsec / 1000000);
}

time_t GetRealTime() {
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  return TimespecToMilliseconds(&time);
}

time_t GetMonotonicTime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  return TimespecToMilliseconds(&time);
}
