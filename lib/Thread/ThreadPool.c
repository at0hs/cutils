/**
 * @file ThreadPool.c
 * @brief スレッドプール
 * @author atohs
 * @date 2024/07/12
 */
#include "Thread/ThreadPool.h"
#include "utilities.h"
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//! ワーカースレッド数を取得できなかった場合の既定値
#define DEFAULT_CONCURRENCY 4

/**
 * @brief タスクキューのノード
 */
struct ThreadPoolTaskNode_t {
  //! タスク本体 (Push 時にコピー)
  ThreadPoolTask_t task;
  //! 次のノード
  struct ThreadPoolTaskNode_t *next;
};

/**
 * @brief ノード連結リストを解放する
 * @param node 先頭ノード
 */
static void FreeNodes(ThreadPoolTaskNode_t *node) {
  while (node) {
    ThreadPoolTaskNode_t *next = node->next;
    free(node);
    node = next;
  }
}

/**
 * @brief キューの先頭からノードを取り出す (mutex 保持中に呼ぶこと)
 * @param self インスタンス
 * @return ノード。キューが空の場合は NULL
 */
static ThreadPoolTaskNode_t *Dequeue(ThreadPool_t *self) {
  ThreadPoolTaskNode_t *node = self->head;
  if (node) {
    self->head = node->next;
    if (self->head == NULL)
      self->tail = NULL;
    self->numTasks--;
  }
  return node;
}

/**
 * @brief 連結済みのノード列をキュー末尾に追加し、ワーカーへ通知する
 * @param self インスタンス
 * @param first 先頭ノード
 * @param last 末尾ノード
 * @param count ノード数
 */
static void EnqueueNodes(ThreadPool_t *self,
                         ThreadPoolTaskNode_t *first,
                         ThreadPoolTaskNode_t *last,
                         uint64_t count) {
  pthread_mutex_lock(&self->mutex);
  if (self->tail)
    self->tail->next = first;
  else
    self->head = first;
  self->tail = last;
  self->numTasks += count;
  if (count == 1)
    pthread_cond_signal(&self->cond);
  else
    pthread_cond_broadcast(&self->cond);
  pthread_mutex_unlock(&self->mutex);
}

/**
 * @brief ワーカースレッド
 * @param arg インスタンス
 * @return NULL
 * @details キューが空かつ shutdown 要求があれば終了する。
 *          そのため shutdown 後も残タスクはすべて処理される。
 */
static void *WorkerThread(void *arg) {
  ThreadPool_t *self = arg;
  while (1) {
    pthread_mutex_lock(&self->mutex);
    while (self->head == NULL && !self->shutdown)
      pthread_cond_wait(&self->cond, &self->mutex);
    ThreadPoolTaskNode_t *node = Dequeue(self);
    pthread_mutex_unlock(&self->mutex);

    if (node == NULL)  // キューが空 && shutdown
      break;
    node->task.function(node->task.arg);
    free(node);
  }
  return NULL;
}

/**
 * @brief ワーカーに終了を要求し、全ワーカーの終了を待つ
 * @param self インスタンス
 * @param numWorkers 生成済みワーカー数
 */
static void StopAndJoinWorkers(ThreadPool_t *self, uint16_t numWorkers) {
  pthread_mutex_lock(&self->mutex);
  self->shutdown = true;
  pthread_cond_broadcast(&self->cond);
  pthread_mutex_unlock(&self->mutex);
  for (uint16_t i = 0; i < numWorkers; i++)
    (void)pthread_join(self->workers[i], NULL);
}

/**
 * @brief 同期オブジェクトとワーカー配列、残タスクを解放し、未初期化状態に戻す
 * @param self インスタンス
 */
static void ReleaseResources(ThreadPool_t *self) {
  FreeNodes(self->head);
  free(self->workers);
  pthread_cond_destroy(&self->cond);
  pthread_mutex_destroy(&self->mutex);
  CLEAR(self);
}

/**
 * @brief プロセスの並行性を取得
 * @return プロセッサ数
 */
static uint32_t GetHardwareConcurrency(void) {
  cpu_set_t cpu_set;
  if (sched_getaffinity(0, sizeof(cpu_set), &cpu_set) == 0) {
    int count = CPU_COUNT(&cpu_set);
    if (count > 0)
      return (uint32_t)count;
  }
  long online = sysconf(_SC_NPROCESSORS_ONLN);
  return online > 0 ? (uint32_t)online : DEFAULT_CONCURRENCY;
}

/**
 * @brief 初期化
 * @param self インスタンス
 * @param numThreads ワーカースレッド数 (0: 論理 CPU 数)
 * @return 結果コード
 */
ThreadPoolReturnCode ThreadPool_Init(ThreadPool_t *self, uint16_t numThreads) {
  if (UNLIKELY(self == NULL))
    return THREAD_POOL_INVALID_ARG;

  CLEAR(self);
  if (numThreads > 0) {
    self->maxNumThreads = numThreads;
  } else {
    uint32_t concurrency = GetHardwareConcurrency();
    self->maxNumThreads =
        concurrency > UINT16_MAX ? UINT16_MAX : (uint16_t)concurrency;
  }

  if (pthread_mutex_init(&self->mutex, NULL) != 0) {
    CLEAR(self);
    return THREAD_POOL_NO_MEMORY;
  }
  if (pthread_cond_init(&self->cond, NULL) != 0) {
    pthread_mutex_destroy(&self->mutex);
    CLEAR(self);
    return THREAD_POOL_NO_MEMORY;
  }

  self->workers = calloc(self->maxNumThreads, sizeof(pthread_t));
  if (UNLIKELY(self->workers == NULL)) {
    ReleaseResources(self);
    return THREAD_POOL_NO_MEMORY;
  }

  for (uint16_t i = 0; i < self->maxNumThreads; i++) {
    if (pthread_create(&self->workers[i], NULL, WorkerThread, self) != 0) {
      StopAndJoinWorkers(self, i);
      ReleaseResources(self);
      return THREAD_POOL_THREAD_CREATE_FAILED;
    }
    // スレッド名は最大 15 文字 (+ NUL)。失敗しても動作には影響しない
    char name[16];
    snprintf(name, sizeof(name), "TPool_%u", (unsigned)i);
    (void)pthread_setname_np(self->workers[i], name);
  }
  return THREAD_POOL_SUCCESS;
}

/**
 * @brief タスクが有効か判定する
 * @param task タスク
 * @return 有効なら true
 */
static inline bool IsValidTask(const ThreadPoolTask_t *task) {
  return task != NULL && task->function != NULL;
}

/**
 * @brief タスクをプッシュ
 * @param self インスタンス
 * @param task タスク
 * @return 結果コード
 */
ThreadPoolReturnCode ThreadPool_Push(ThreadPool_t *self,
                                     const ThreadPoolTask_t *task) {
  if (UNLIKELY(self == NULL || !IsValidTask(task)))
    return THREAD_POOL_INVALID_ARG;
  if (UNLIKELY(self->workers == NULL))
    return THREAD_POOL_NOT_INITIALIZED;

  ThreadPoolTaskNode_t *node = malloc(sizeof(*node));
  if (UNLIKELY(node == NULL))
    return THREAD_POOL_NO_MEMORY;
  node->task = *task;
  node->next = NULL;
  EnqueueNodes(self, node, node, 1);
  return THREAD_POOL_SUCCESS;
}

/**
 * @brief タスクをまとめてプッシュ (全件 or 0 件)
 * @param self インスタンス
 * @param tasks タスク集合
 * @param numTasks 数
 * @return 結果コード
 */
ThreadPoolReturnCode ThreadPool_PushTasks(ThreadPool_t *self,
                                          const ThreadPoolTask_t *tasks[],
                                          uint64_t numTasks) {
  if (UNLIKELY(self == NULL || tasks == NULL || numTasks == 0))
    return THREAD_POOL_INVALID_ARG;
  for (uint64_t i = 0; i < numTasks; i++) {
    if (UNLIKELY(!IsValidTask(tasks[i])))
      return THREAD_POOL_INVALID_ARG;
  }
  if (UNLIKELY(self->workers == NULL))
    return THREAD_POOL_NOT_INITIALIZED;

  // ロック外でノード列を組み立て、一度のロックで連結する
  ThreadPoolTaskNode_t *first = NULL, *last = NULL;
  for (uint64_t i = 0; i < numTasks; i++) {
    ThreadPoolTaskNode_t *node = malloc(sizeof(*node));
    if (UNLIKELY(node == NULL)) {
      FreeNodes(first);
      return THREAD_POOL_NO_MEMORY;
    }
    node->task = *tasks[i];
    node->next = NULL;
    if (last)
      last->next = node;
    else
      first = node;
    last = node;
  }
  EnqueueNodes(self, first, last, numTasks);
  return THREAD_POOL_SUCCESS;
}

/**
 * @brief インスタンスを破棄
 * @param self インスタンス
 * @param isWait true: 残タスクを処理してから終了 / false: 残タスクを破棄して終了
 */
void ThreadPool_Destroy(ThreadPool_t *self, bool isWait) {
  if (UNLIKELY(self == NULL || self->workers == NULL))
    return;  // NULL / 未初期化 / 二重 Destroy

  if (!isWait) {
    pthread_mutex_lock(&self->mutex);
    ThreadPoolTaskNode_t *pending = self->head;
    self->head = self->tail = NULL;
    self->numTasks = 0;
    pthread_mutex_unlock(&self->mutex);
    FreeNodes(pending);
  }
  StopAndJoinWorkers(self, self->maxNumThreads);
  ReleaseResources(self);
}

/**
 * @brief タスク数を取得
 * @param self インスタンス
 * @return タスク数
 */
uint64_t ThreadPool_GetNumTasks(ThreadPool_t *self) {
  if (UNLIKELY(self == NULL || self->workers == NULL))
    return 0;
  pthread_mutex_lock(&self->mutex);
  uint64_t numTasks = self->numTasks;
  pthread_mutex_unlock(&self->mutex);
  return numTasks;
}
