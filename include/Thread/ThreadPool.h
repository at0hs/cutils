/**
 * @file ThreadPool.h
 * @brief スレッドプール
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ThreadPool API の戻り値
 */
typedef enum ThreadPoolReturnCode {
  THREAD_POOL_SUCCESS = 0,                /**< 操作成功 */
  THREAD_POOL_INVALID_ARG = -1,           /**< 引数が NULL または不正 */
  THREAD_POOL_NOT_INITIALIZED = -2,       /**< 未初期化 / Destroy 済み */
  THREAD_POOL_NO_MEMORY = -3,             /**< メモリ確保の失敗 */
  THREAD_POOL_THREAD_CREATE_FAILED = -4,  /**< ワーカースレッド生成の失敗 */
} ThreadPoolReturnCode;

/**
 * @brief スレッドプールで実行するタスク
 */
typedef struct ThreadPoolTask_t {
  //! 実行する関数
  void (*function)(void *arg);
  //! 関数に渡す引数
  void *arg;
} ThreadPoolTask_t;

//! タスクキューのノード (内部用)
typedef struct ThreadPoolTaskNode_t ThreadPoolTaskNode_t;

/**
 * @brief スレッドプール
 */
typedef struct ThreadPool_t {
  //! @name Private
  //! @{

  //! タスクキューの先頭
  ThreadPoolTaskNode_t *head;
  //! タスクキューの末尾
  ThreadPoolTaskNode_t *tail;
  //! キュー内の未処理タスク数
  uint64_t numTasks;
  //! ワーカースレッド配列 (NULL の場合は未初期化 / Destroy 済み)
  pthread_t *workers;
  //! 生成したワーカースレッド数
  uint16_t maxNumThreads;
  //! true の場合、キューが空になったワーカーは終了する
  bool shutdown;
  //! タスクキューを保護するミューテックス
  pthread_mutex_t mutex;
  //! 新規タスク到着 / 終了要求を通知する条件変数
  pthread_cond_t cond;

  //! @}
} ThreadPool_t;

/**
 * @brief スレッドプールを初期化する
 * @param self インスタンス
 * @param numThreads 起動するワーカースレッド数。0 を指定すると論理 CPU 数を使用する
 * @retval THREAD_POOL_SUCCESS 成功
 * @retval THREAD_POOL_INVALID_ARG self が NULL
 * @retval THREAD_POOL_NO_MEMORY メモリ確保の失敗
 * @retval THREAD_POOL_THREAD_CREATE_FAILED ワーカースレッド生成の失敗
 * @note 失敗した場合、生成済みのリソースはすべて解放され self は未初期化状態になる
 */
extern ThreadPoolReturnCode ThreadPool_Init(ThreadPool_t *self,
                                            uint16_t numThreads);

/**
 * @brief タスクをキューに追加する
 * @param self インスタンス
 * @param task 追加するタスク (内容はコピーされる)
 * @retval THREAD_POOL_SUCCESS 成功
 * @retval THREAD_POOL_INVALID_ARG self / task / task->function が NULL
 * @retval THREAD_POOL_NOT_INITIALIZED 未初期化 / Destroy 済み
 * @retval THREAD_POOL_NO_MEMORY メモリ確保の失敗
 */
extern ThreadPoolReturnCode ThreadPool_Push(ThreadPool_t *self,
                                            const ThreadPoolTask_t *task);

/**
 * @brief 複数のタスクをまとめてキューに追加する
 * @param self インスタンス
 * @param tasks タスクポインタの配列
 * @param numTasks タスク数
 * @retval THREAD_POOL_SUCCESS 成功
 * @retval THREAD_POOL_INVALID_ARG 引数が NULL / numTasks が 0 / いずれかのタスクが不正
 * @retval THREAD_POOL_NOT_INITIALIZED 未初期化 / Destroy 済み
 * @retval THREAD_POOL_NO_MEMORY メモリ確保の失敗
 * @note 全件追加されるか、1 件も追加されないかのどちらかになる
 */
extern ThreadPoolReturnCode ThreadPool_PushTasks(
    ThreadPool_t *self, const ThreadPoolTask_t *tasks[], uint64_t numTasks);

/**
 * @brief スレッドプールを破棄する
 * @param self インスタンス
 * @param isWait true の場合はキュー内の残タスクをすべて処理してから終了する。
 *               false の場合は残タスクを破棄し、実行中のタスクの完了を待って終了する
 * @note self が NULL / 未初期化 / Destroy 済みの場合は何もしない
 */
extern void ThreadPool_Destroy(ThreadPool_t *self, bool isWait);

/**
 * @brief キューに積まれている未処理タスク数を返す
 * @param self インスタンス
 * @return 未処理タスク数 (self が NULL / 未初期化の場合は 0)
 */
extern uint64_t ThreadPool_GetNumTasks(ThreadPool_t *self);

#ifdef __cplusplus
}
#endif
