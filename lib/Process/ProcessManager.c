/**
 * @file ProcessManager.c
 * @brief プロセスマネージャー (内部モジュール)
 * @author atohs
 * @date 2024/08/29
 */
#include "ProcessManager.h"
#include "utilities.h"
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <wait.h>

/**
 * @brief プロセス登録エントリ
 */
typedef struct ProcessStartInfo {
  /** @brief 監視対象プロセスの PID */
  pid_t pid;
  /** @brief 終了時コールバックに渡すユーザーデータ */
  void *data;
} ProcessStartInfo;

/**
 * @brief ProcessManager シングルトンの内部状態
 */
typedef struct ProcessManager {
  /** @brief PID をキーとするプロセステーブル (GHashTable) */
  GHashTable *processes;
  /** @brief プロセステーブルを保護する mutex */
  pthread_mutex_t mutex;
  /** @brief 子プロセス終了時に呼び出すコールバック */
  ProcessExitedEventHandler onExited;
} ProcessManager;

#define INITIALIZE_NUM_PROCESSES 64
#define LOCK(m, CODE)                                                          \
  {                                                                            \
    pthread_mutex_lock((m));                                                   \
    CODE;                                                                      \
    pthread_mutex_unlock((m));                                                 \
  }

static ProcessManager _processManager;
static atomic_bool initialized = false;
static inline ProcessManager *getMyself() { return &_processManager; }

/**
 * @brief 2 つの PID が等しいか比較する
 */
static gboolean AreEquals(pid_t a, pid_t b) { return a == b; }

/**
 * @brief GHashTable 向けのキー等値比較ラッパー
 */
static gboolean AreEqualsWrapper(gconstpointer a, gconstpointer b) {
  return AreEquals((pid_t)GPOINTER_TO_INT(a), (pid_t)GPOINTER_TO_INT(b));
}

/**
 * @brief PID からハッシュ値を生成する
 */
static guint GenerateHash(pid_t pid) { return pid; }

/**
 * @brief GHashTable 向けのハッシュ関数ラッパー
 */
static guint GenerateHashWrapper(gconstpointer key) {
  return GenerateHash((pid_t)GPOINTER_TO_INT(key));
}

/**
 * @brief SIGCHLD シグナルハンドラ
 *
 * 子プロセスが終了すると呼ばれ、プロセステーブルから対応エントリを取得して
 * onExited コールバックを呼び出す。
 * @param signalNumber シグナル番号 (常に SIGCHLD)
 * @param signalInfo シグナル発生元の情報 (si_pid, si_status など)
 * @param ucontext 未使用
 */
static void sigchldHandler(int signalNumber, siginfo_t *signalInfo,
                           void *ucontext) {
  ProcessManager *self = getMyself();
  void *data;
  LOCK(&self->mutex, {
    data = g_hash_table_lookup(self->processes,
                               GINT_TO_POINTER(signalInfo->si_pid));
  });
  // printf("Process %d exited with code %d\n", signalInfo->si_pid,
  // signalInfo->si_status);
  if (!data || !self->onExited) {
    return;
  }
  self->onExited(signalInfo->si_pid, signalInfo->si_status, data);
}

/**
 * @brief SIGCHLD ハンドラを sigaction で登録する
 */
static void HandleSigchld() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = sigchldHandler;
  sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);
}

void ProcessManager_Init(ProcessExitedEventHandler onExited) {
  if (initialized)
    return;
  initialized = true;
  ProcessManager *self = getMyself();
  CLEAR(self);
  self->onExited = onExited;
  self->processes = g_hash_table_new((GHashFunc)GenerateHashWrapper,
                                     (GEqualFunc)AreEqualsWrapper);
  pthread_mutex_init(&self->mutex, NULL);
  HandleSigchld();
}

void ProcessManager_Destroy() {
  ProcessManager *self = getMyself();
  g_hash_table_destroy(self->processes);
  CLEAR(self);
}

void ProcessManager_Register(pid_t pid, void *data) {
  ProcessManager *self = getMyself();
  LOCK(&self->mutex,
       { g_hash_table_insert(self->processes, GINT_TO_POINTER(pid), data); });
}

void ProcessManager_Unregister(pid_t pid) {
  ProcessManager *self = getMyself();
  LOCK(&self->mutex,
       { g_hash_table_remove(self->processes, GINT_TO_POINTER(pid)); });
  return;
}
