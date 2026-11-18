/**
 * @file ThreadPool.h
 * @brief スレッドプール
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#include <stdbool.h>
#ifdef ATOMIC
#undef ATOMIC
#endif

#ifdef __cplusplus
#include <atomic>
#define ATOMIC(t) std::atomic<t>
extern "C" {
#else
#define ATOMIC(t) _Atomic t
#endif

#include <glib.h>
#include <stdint.h>

/**
 * @brief スレッドプールで実行するタスク
 */
typedef struct ThreadPoolTask_t {
  //! 実行する関数
  void (*function)(void *arg);
  //! 関数に渡す引数
  void *arg;
} ThreadPoolTask_t;

/**
 * @brief スレッドプール
 */
typedef struct ThreadPool_t {
  //! @name Private
  //! @{

  //! タスクキュー
  GQueue *tasks;
  //! ワーカースレッド配列
  GThread **workers;
  //! 生成するワーカースレッド数
  uint16_t maxNumThreads;
  //! タスクキューを保護するミューテックス
  GMutex mutex;
  //! 新規タスク到着を通知する条件変数
  GCond cond;

  //! @}
} ThreadPool_t;

/**
 * @brief スレッドプールを初期化する
 * @param self インスタンス
 * @param numThreads 起動するワーカースレッド数。0 を指定すると論理 CPU 数を使用する
 */
extern void ThreadPool_Init(ThreadPool_t *self, uint16_t numThreads);

/**
 * @brief タスクをキューに追加する
 * @param self インスタンス
 * @param task 追加するタスク
 * @return 0: 成功, -1: 失敗
 */
extern int ThreadPool_Push(ThreadPool_t *self, const ThreadPoolTask_t *task);

/**
 * @brief 複数のタスクをまとめてキューに追加する
 * @param self インスタンス
 * @param tasks タスクポインタの配列
 * @param numTasks タスク数
 * @return 0: 成功, -1: 失敗（いずれか 1 件でも失敗した場合）
 */
extern int ThreadPool_PushTasks(ThreadPool_t *self,
                                const ThreadPoolTask_t *tasks[],
                                uint64_t numTasks);

/**
 * @brief スレッドプールを破棄する
 * @param self インスタンス
 * @param isWait true の場合はキュー内の残タスクをすべて処理してから終了する。
 *               false の場合は残タスクを破棄して即座に終了する
 */
extern void ThreadPool_Destroy(ThreadPool_t *self, bool isWait);

/**
 * @brief キューに積まれている未処理タスク数を返す
 * @param self インスタンス
 * @return 未処理タスク数
 */
extern uint64_t ThreadPool_GetNumTasks(ThreadPool_t *self);

#ifdef __cplusplus
}
#endif
