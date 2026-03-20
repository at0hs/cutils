/**
 * @file ThreadPool.c
 * @brief スレッドプール
 * @author atohs
 * @date 2024/07/12
 */
#include "Thread/ThreadPool.h"
#include "utilities.h"
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MUTEX_INIT(mutex) (g_mutex_init((mutex)))
#define MUTEX_LOCK(mutex) (g_mutex_lock((mutex)))
#define MUTEX_UNLOCK(mutex) (g_mutex_unlock((mutex)))
#define MUTEX_DESTROY(mutex) (g_mutex_clear((mutex)))

#define COND_INIT(cond) (g_cond_init((cond)))
#define COND_SIGNAL(cond) (g_cond_signal((cond)))
#define COND_WAIT(cond, mutex) (g_cond_wait((cond), (mutex)))
#define COND_BROADCAST(cond) (g_cond_broadcast((cond)))
#define COND_DESTROY(cond) (g_cond_clear((cond)))

#define QUEUE_POP g_queue_pop_head
#define QUEUE_PUSH g_queue_push_tail

static void _ExitWorker(void *);
//! ワーカースレッドを終了させる
static const ThreadPoolTask_t exitTask = {.function = _ExitWorker, .arg = NULL};

/**
 * @brief 新たなタスクが来るのを待つ
 * @param self インスタンス
 * @return タスク
 */
static ThreadPoolTask_t *WaitForNewTask(ThreadPool_t *self) {
  ThreadPoolTask_t *task;
  MUTEX_LOCK(&self->mutex);
  while ((task = QUEUE_POP(self->tasks)) == NULL)
    COND_WAIT(&self->cond, &self->mutex);
  MUTEX_UNLOCK(&self->mutex);
  return task;
}

/**
 * @brief スレッドの終了
 * @param arg 使用しない
 */
static void _ExitWorker(void *arg) {
  (void)arg;
  g_thread_exit(NULL);
}

/**
 * @brief ワーカースレッドを停止する
 * @param self インスタンス
 */
static inline void StopWorkers(ThreadPool_t *self) {
  for (uint16_t i = 0; i < self->maxNumThreads; i++) {
    MUTEX_LOCK(&self->mutex);
    QUEUE_PUSH(self->tasks, (void *)&exitTask);
    COND_SIGNAL(&self->cond);
    MUTEX_UNLOCK(&self->mutex);
  }
}

/**
 * @brief ワーカースレッド
 * @param arg インスタンス
 * @return 戻らない
 */
static gpointer WorkerThread(gpointer arg) {
  ThreadPool_t *self = arg;
  while (1) {
    ThreadPoolTask_t *task = WaitForNewTask(self);
    if (task) {
      task->function(task->arg);
      free(task);
    }
  }
  __builtin_unreachable();
}

/**
 * @brief ワーカーが停止するのを待つ
 * @param self インスタンス
 */
static void WaitForWorkersExited(ThreadPool_t *self) {
  for (uint16_t i = 0; i < self->maxNumThreads; i++) {
    GThread *thread = self->workers[i];
    if (thread) {
      (void)g_thread_join(thread);
    }
  }
}

/**
 * @brief ワーカースレッドを強制停止する（未処理タスクをドレインしてから終了）
 * @param self インスタンス
 */
static void AbortWorkers(ThreadPool_t *self) {
  MUTEX_LOCK(&self->mutex);
  ThreadPoolTask_t *task;
  while ((task = QUEUE_POP(self->tasks)) != NULL)
    free(task);
  MUTEX_UNLOCK(&self->mutex);
  StopWorkers(self);
}

/**
 * @brief プロセスの並行性を取得
 * @return プロセッサ数
 */
static uint32_t GetHardwareConcurrency() {
  cpu_set_t cpu_set;
  if (sched_getaffinity(getpid(), sizeof(cpu_set), &cpu_set) != 0)
    return 4;
  uint32_t count = CPU_COUNT(&cpu_set);
  return count > 0 ? count : 4;
}

/**
 * @brief 初期化
 * @param self インスタンス
 * @param numThreads ワーカースレッド数
 */
void ThreadPool_Init(ThreadPool_t *self, uint16_t numThreads) {
  g_return_if_fail(self);

  CLEAR(self);
  if (numThreads > 0) {
    self->maxNumThreads = numThreads;
  } else {
    self->maxNumThreads = GetHardwareConcurrency();
  }

  self->tasks = g_queue_new();
  MUTEX_INIT(&self->mutex);
  COND_INIT(&self->cond);

  self->workers =
      (GThread **)g_malloc0_n(self->maxNumThreads, sizeof(GThread *));
  for (uint16_t i = 0; i < self->maxNumThreads; i++) {
    char name[20];
    snprintf(name, sizeof(name), "ThreadPool_%d", i);
    self->workers[i] = g_thread_new(name, WorkerThread, self);
  }
}

/**
 * @brief タスクをプッシュ
 * @param self インスタンス
 * @param task タスク
 * @return 0: ok
 */
int ThreadPool_Push(ThreadPool_t *self, const ThreadPoolTask_t *task) {
  g_return_val_if_fail(self, -1);
  g_return_val_if_fail(task, -1);
  ThreadPoolTask_t *_task = g_malloc(sizeof(ThreadPoolTask_t));
  *_task = *task;
  MUTEX_LOCK(&self->mutex);
  QUEUE_PUSH(self->tasks, _task);
  COND_SIGNAL(&self->cond);
  MUTEX_UNLOCK(&self->mutex);
  return 0;
}

/**
 * @brief タスクをプッシュ
 * @param self インスタンス
 * @param tasks タスク集合
 * @param numTasks 数
 * @return 0: ok
 */
int ThreadPool_PushTasks(ThreadPool_t *self, const ThreadPoolTask_t *tasks[],
                         uint64_t numTasks) {
  g_return_val_if_fail(self, -1);
  g_return_val_if_fail(tasks, -1);
  g_return_val_if_fail(numTasks, -1);

  for (uint64_t i = 0; i < numTasks; i++) {
    if (ThreadPool_Push(self, tasks[i]) != 0) {
      return -1;
    }
  }
  return 0;
}

/**
 * @brief インスタンスを破棄
 * @param self インスタンス
 * @param isWait trueの場合は残タスクを処理してから終了、falseの場合は残タスクを破棄して終了
 */
void ThreadPool_Destroy(ThreadPool_t *self, bool isWait) {
  g_return_if_fail(self);
  if (isWait) {
    StopWorkers(self);
  } else {
    AbortWorkers(self);
  }
  WaitForWorkersExited(self);
  g_queue_free(self->tasks);
  if (self->workers)
    g_free((gpointer)self->workers);
  COND_DESTROY(&self->cond);
  MUTEX_DESTROY(&self->mutex);
  CLEAR(self);
}

/**
 * @brief タスク数を取得
 * @param self インスタンス
 * @return タスク数
 */
uint64_t ThreadPool_GetNumTasks(ThreadPool_t *self) {
  g_return_val_if_fail(self, 0);
  MUTEX_LOCK(&self->mutex);
  uint64_t numTasks = g_queue_get_length(self->tasks);
  MUTEX_UNLOCK(&self->mutex);
  return numTasks;
}
