/**
 * @file Process.c
 * @brief プロセスの起動・監視・リソース管理の実装
 * @details
 * fork/execve によるサブプロセスの起動、標準出力・標準エラーのファイルキャプチャ、
 * 待機スレッドによる終了検知、rusage によるリソース計測を提供する。
 *
 * 終了検知は子プロセスごとに専用の待機スレッドが wait4() でブロックして行う。
 * SIGCHLD ハンドラは使用しないため、
 * - アプリケーション側の SIGCHLD ハンドラを上書きしない
 * - シグナルハンドラ内で非 async-signal-safe な関数を呼ぶことがない
 * - fork() 直後に子が終了しても取りこぼさない
 * - 複数の子が同時に終了しても SIGCHLD の合流で回収漏れが起きない
 * という性質を持つ。
 * @author atohs
 * @date 2024/08/29
 */
#include "Process/Process.h"
#include "utilities.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

//! @brief ワークスペースディレクトリのテンプレート (mkdtemp 用)
#define WORKSPACE_TEMPLATE "/tmp/cutils_process_XXXXXX"
#define SHELL_PATH         "/bin/sh"
#define SHELL_NAME         "sh"
#define OPTION_COMMAND     "-c"
//! @brief execve 失敗時の終了コード (シェルの "command not found" と同じ慣例)
#define PROCESS_FAILED       (127)
#define STANDARD_OUTPUT_FILE "standard_output"
#define STANDARD_ERROR_FILE  "standard_error"
//! @brief 出力ファイルパスのバッファサイズ
#define PATH_BUFFER_SIZE (64)

_Static_assert(sizeof(WORKSPACE_TEMPLATE) <= sizeof(((Process *)0)->workspace),
               "workspace buffer is too small");

//! @name static methods
//! @{

/**
 * @brief プロセスごとのワークスペースディレクトリを作成する
 *
 * mkdtemp() により推測困難な名前でパーミッション 0700 のディレクトリを作る。
 * 複数のプログラムが同時にこのライブラリを使用しても衝突しない。
 * @param self Process インスタンス
 */
static inline void CreateWorkspace(Process *self) {
  snprintf(self->workspace, sizeof(self->workspace), "%s", WORKSPACE_TEMPLATE);
  if (!mkdtemp(self->workspace)) {
    self->workspace[0] = '\0';
  }
}

/**
 * @brief ワークスペース内のファイルパスを生成する
 * @param self     Process インスタンス
 * @param fileName ファイル名
 * @param[out] buffer 格納先バッファ (PATH_BUFFER_SIZE バイト)
 */
static inline void MakePath(const Process *self,
                            const char *fileName,
                            char buffer[PATH_BUFFER_SIZE]) {
  snprintf(buffer, PATH_BUFFER_SIZE, "%s/%s", self->workspace, fileName);
}

/**
 * @brief ワークスペースディレクトリと一時ファイルを削除する
 * @param self Process インスタンス
 */
static inline void RemoveWorkspace(Process *self) {
  if (self->workspace[0] == '\0') {
    return;
  }
  char path[PATH_BUFFER_SIZE] = {0};
  MakePath(self, STANDARD_OUTPUT_FILE, path);
  remove(path);
  MakePath(self, STANDARD_ERROR_FILE, path);
  remove(path);
  rmdir(self->workspace);
}

/**
 * @brief プロセスが起動済みか判定する
 * @param self Process インスタンス
 * @return fork() に成功していれば true
 */
static inline bool IsStarted(const Process *self) {
  return self->id > 0;
}

/**
 * @brief 終了状態を mutex で保護して読み出す
 * @param self Process インスタンス
 * @return 終了ステータスが確定していれば true
 */
static bool IsTerminated(Process *self) {
  pthread_mutex_lock(&self->mutex);
  bool terminated = self->terminated;
  pthread_mutex_unlock(&self->mutex);
  return terminated;
}

/**
 * @brief 子プロセスの終了を待機するスレッド
 *
 * wait4() で子プロセスを回収し、終了ステータスとリソース使用量を保存してから
 * onExited を呼び出し、最後に待機中のスレッドへ通知する。
 * @param arg Process インスタンス
 * @return 常に NULL
 */
static void *WaiterThread(void *arg) {
  Process *self = (Process *)arg;
  int status = 0;
  struct rusage usage = {0};
  pid_t result;
  do {
    result = wait4(self->id, &status, 0, &usage);
  } while (result < 0 && errno == EINTR);
  const double exitTime = (double)GetRealTime() / 1000.0;

  pthread_mutex_lock(&self->mutex);
  if (result == self->id) {
    self->status = status;
    self->usage = usage;
  }
  self->exitTime = exitTime;
  self->terminated = true;  // ここ以降 status / usage は確定している
  pthread_mutex_unlock(&self->mutex);

  if (self->onExited) {
    self->onExited(self);
  }

  pthread_mutex_lock(&self->mutex);
  self->exitNotified = true;
  pthread_cond_broadcast(&self->condition);
  pthread_mutex_unlock(&self->mutex);
  return NULL;
}

/**
 * @brief 子プロセスの標準出力・標準エラーをファイルへリダイレクトする
 * @param standardOutput 標準出力の出力先ファイルパス
 * @param standardError  標準エラーの出力先ファイルパス
 */
static void CaptureOutput(const char *standardOutput,
                          const char *standardError) {
  int outputFd = open(standardOutput, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (outputFd >= 0) {
    dup2(outputFd, STDOUT_FILENO);
    close(outputFd);
  }
  int errorFd = open(standardError, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (errorFd >= 0) {
    dup2(errorFd, STDERR_FILENO);
    close(errorFd);
  }
}

/**
 * @brief timeval を秒単位の double に変換する (ミリ秒精度)
 * @param time 変換元
 * @return 秒
 */
static inline double TimevalToDouble(const struct timeval *time) {
  return (double)time->tv_sec +
         ((double)((long)(time->tv_usec / 1000)) / 1000.0);
}

/**
 * @brief 子プロセス側の処理 (fork 後に呼ばれ、戻らない)
 * @param self       Process インスタンス
 * @param outputPath 標準出力のキャプチャ先 (fork 前に生成済み)
 * @param errorPath  標準エラーのキャプチャ先 (fork 前に生成済み)
 */
static void ChildProcess(const Process *self,
                         const char *outputPath,
                         const char *errorPath) {
  const char *newArgv[] = {
      SHELL_NAME, OPTION_COMMAND, self->info.commandLine, NULL};
  if (!self->info.disableOutputCapture) {
    CaptureOutput(outputPath, errorPath);
  }
  execve(SHELL_PATH, (char *const *)newArgv, self->info.environment);
  _exit(PROCESS_FAILED);
}

//! @}

void Process_Init(Process *self, const ProcessStartInfo *info) {
  if (UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);

  self->status = -1;
  self->id = -1;
  if (info) {
    self->info = *info;
  }

  CreateWorkspace(self);

  pthread_mutex_init(&self->mutex, NULL);

  pthread_condattr_t conditionAttribute;
  pthread_condattr_init(&conditionAttribute);
  pthread_condattr_setclock(&conditionAttribute, CLOCK_MONOTONIC);
  pthread_cond_init(&self->condition, &conditionAttribute);
  pthread_condattr_destroy(&conditionAttribute);
}

void Process_Execute(Process *self, const ProcessStartInfo *info) {
  if (UNLIKELY(!self || !info || !info->commandLine)) {
    return;
  }
  Process_Init(self, info);
  Process_Start(self);
}

void Process_Destroy(Process *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  if (self->waiterStarted) {
    // 待機スレッドが self を参照し続けないよう、必ず終了させてから破棄する。
    // まだ動いている子プロセスは強制終了する。
    pthread_mutex_lock(&self->mutex);
    if (!self->terminated) {
      kill(self->id, SIGKILL);
    }
    pthread_mutex_unlock(&self->mutex);
    pthread_join(self->waiter, NULL);
  }
  pthread_cond_destroy(&self->condition);
  pthread_mutex_destroy(&self->mutex);
  RemoveWorkspace(self);
  CLEAR(self);
}

//! @name properties
//! @{

bool Process_HasExited(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return false;
  }
  if (!IsTerminated(self)) {
    return false;
  }
  return WIFEXITED(self->status);
}

int32_t Process_ExitCode(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  if (!IsTerminated(self)) {
    return -1;
  }
  if (!WIFEXITED(self->status)) {
    return -1;
  }
  return WEXITSTATUS(self->status);
}

double Process_TotalProcessorTime(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  if (!IsTerminated(self)) {
    return -1;
  }
  return TimevalToDouble(&self->usage.ru_utime) +
         TimevalToDouble(&self->usage.ru_stime);
}

double Process_UserProcessorTime(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  if (!IsTerminated(self)) {
    return -1;
  }
  return TimevalToDouble(&self->usage.ru_utime);
}

double Process_StartTime(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  return self->startTime;
}

double Process_ExitTime(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  if (!IsTerminated(self)) {
    return -1;
  }
  return self->exitTime;
}

pid_t Process_Id(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return -1;
  }
  return self->id;
}

/**
 * @brief キャプチャした出力ファイルを ProcessOutput に設定する共通処理
 * @param self     Process インスタンス
 * @param fileName 出力ファイル名
 * @param[out] result 格納先
 */
static void GetOutput(Process *self,
                      const char *fileName,
                      ProcessOutput *result) {
  if (UNLIKELY(!result)) {
    return;
  }
  CLEAR(result);  // 早期リターン時も size == 0 として扱えるようにする
  if (UNLIKELY(!self || !IsStarted(self))) {
    return;
  }
  if (self->info.disableOutputCapture) {
    return;
  }
  if (!IsTerminated(self)) {
    return;
  }
  char path[PATH_BUFFER_SIZE] = {0};
  MakePath(self, fileName, path);
  ProcessOutput_Init(result, path);
}

void Process_StandardOutput(Process *self, ProcessOutput *result) {
  GetOutput(self, STANDARD_OUTPUT_FILE, result);
}

void Process_StandardError(Process *self, ProcessOutput *result) {
  GetOutput(self, STANDARD_ERROR_FILE, result);
}
//! @}

//! @name public methods
//! @{

void Process_Start(Process *self) {
  if (UNLIKELY(!self || !self->info.commandLine)) {
    return;
  }
  if (IsStarted(self)) {
    return;  // 1 つのインスタンスで起動できるのは 1 回だけ
  }
  // fork 後の子プロセスでは最小限の処理にするため、パスは事前に生成しておく
  char outputPath[PATH_BUFFER_SIZE] = {0};
  char errorPath[PATH_BUFFER_SIZE] = {0};
  MakePath(self, STANDARD_OUTPUT_FILE, outputPath);
  MakePath(self, STANDARD_ERROR_FILE, errorPath);

  pid_t pid = fork();
  if (pid < 0) {
    self->id = -1;
    return;
  }
  if (pid == 0) {
    ChildProcess(self, outputPath, errorPath);  // no return
  }
  self->id = pid;
  self->startTime = (double)GetRealTime() / 1000.0;
  if (pthread_create(&self->waiter, NULL, WaiterThread, self) == 0) {
    self->waiterStarted = true;
  } else {
    // 待機スレッドを作れない場合はこの場で回収する
    WaiterThread(self);
  }
}

void Process_Wait(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return;
  }
  pthread_mutex_lock(&self->mutex);
  while (!self->exitNotified) {
    pthread_cond_wait(&self->condition, &self->mutex);
  }
  pthread_mutex_unlock(&self->mutex);
}

bool Process_WaitFor(Process *self, time_t milliSeconds) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return false;
  }
  struct timespec deadline;
  clock_gettime(CLOCK_MONOTONIC, &deadline);
  deadline.tv_sec += milliSeconds / 1000;
  deadline.tv_nsec += (long)(milliSeconds % 1000) * 1000000L;
  if (deadline.tv_nsec >= 1000000000L) {
    deadline.tv_sec++;
    deadline.tv_nsec -= 1000000000L;
  }

  pthread_mutex_lock(&self->mutex);
  int ret = 0;
  while (!self->exitNotified && ret != ETIMEDOUT) {
    ret = pthread_cond_timedwait(&self->condition, &self->mutex, &deadline);
  }
  bool exited = self->exitNotified;
  pthread_mutex_unlock(&self->mutex);
  return exited;
}

void Process_PrintSummary(Process *self) {
  if (UNLIKELY(!self || !IsStarted(self))) {
    return;
  }
  if (!Process_HasExited(self)) {
    printf("Process %d is still running\n", self->id);
    return;
  }
  printf("Process %d exited with code %d\n", self->id, Process_ExitCode(self));
  printf("  Total processor time: %.3fs\n", Process_TotalProcessorTime(self));
  printf("  User processor time: %.3fs\n", Process_UserProcessorTime(self));
  printf("  Elapsed time: %.3fs\n", self->exitTime - self->startTime);
}

//! @}
