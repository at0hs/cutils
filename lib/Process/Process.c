/**
 * @file Process.c
 * @brief プロセスの起動・監視・リソース管理の実装
 * @details
 * fork/execve によるサブプロセスの起動、標準出力・標準エラーのファイルキャプチャ、
 * ProcessManager を介した非同期終了検知、rusage によるリソース計測を提供する。
 * @author atohs
 * @date 2024/08/29
 */
#include "Process/Process.h"
#include "ProcessManager.h"
#include "utilities.h"
#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <wait.h>

atomic_int processCount = 0;
atomic_bool initialized = false;

#define ROOT "/tmp/process"
#define SHELL_PATH "/bin/sh"
#define SHELL_NAME "sh"
#define OPTION_COMMAND "-c"
#define PROCESS_FAILED (-255)
#define STANDARD_OUTPUT_FILE "standard_output"
#define STANDARD_ERROR_FILE "standard_error"

//! @name static methods
//! @{

/**
 * @brief ワークスペースのルートディレクトリを作成する
 * @note ディレクトリが既に存在する場合 (EEXIST) はエラーとしない。
 *       それ以外の失敗は無視される。
 */
static inline void CreateRootDirectory() {
  if (mkdir(ROOT, 0777) != 0 && errno != EEXIST) {
    // directory creation failed
  }
}

/**
 * @brief プロセス固有のワークスペースディレクトリを作成する
 * @param self Process インスタンス。workspace フィールドにパスが設定される
 * @note パスは ROOT/<processCount> の形式。processCount はアトミックにインクリメントされる。
 */
static inline void CreateWorkspace(Process *self) {
  sprintf(self->workspace, "%s/%d", ROOT, processCount++);
  mkdir(self->workspace, 0777);
}

/**
 * @brief ワークスペースディレクトリとその中のキャプチャファイルを削除する
 * @param self Process インスタンス
 * @note 標準出力ファイル・標準エラーファイルを削除した後、ディレクトリを削除する。
 */
static inline void RemoveWorkspace(Process *self) {
  char path[64] = {0};

  sprintf(path, "%s/" STANDARD_OUTPUT_FILE, self->workspace);
  remove(path);

  sprintf(path, "%s/" STANDARD_ERROR_FILE, self->workspace);
  remove(path);

  rmdir(self->workspace);
}

/**
 * @brief 子プロセス終了時に ProcessManager から呼ばれる共通ハンドラ
 * @param pid 終了した子プロセスの PID
 * @param exitStatus 終了ステータス (未使用; wait4 で取得)
 * @param data Process インスタンスへのポインタ
 * @details
 * - onExited コールバックを呼び出す。
 * - Process_Wait() が先に terminated を立てていた場合は wait4 をスキップする。
 * - そうでなければ wait4 でリソース使用量を回収してから ProcessManager の登録を解除する。
 */
static void CommonExitedHandler(pid_t pid, int32_t exitStatus, void *data) {
  Process *self = (Process *)data;

  self->exitTime = (double)GetRealTime() / 1000.0;
  if (self->onExited) {
    self->onExited(self);
  }
  pthread_mutex_lock(&self->mutex);
  if (self->terminated) {
    pthread_mutex_unlock(&self->mutex);
    ProcessManager_Unregister(pid);
    return;
  }
  self->terminated = true;
  pthread_mutex_unlock(&self->mutex);
  wait4(self->id, &self->status, 0, &self->usage);
  ProcessManager_Unregister(pid);
}

/**
 * @brief 子プロセスの標準出力・標準エラーをファイルにリダイレクトする
 * @param standardOutput リダイレクト先の標準出力ファイルパス。NULL の場合はリダイレクトしない
 * @param standardError  リダイレクト先の標準エラーファイルパス。NULL の場合はリダイレクトしない
 * @note fork 後の子プロセス側でのみ呼び出すこと。
 */
static void CaptureOutput(const char *standardOutput,
                          const char *standardError) {
  if (standardOutput) {
    int outputFd = open(standardOutput, O_RDWR | O_CREAT | O_TRUNC, 0777);
    dup2(outputFd, STDOUT_FILENO);
    close(outputFd);
  }
  if (standardError) {
    int errorFd = open(standardError, O_RDWR | O_CREAT | O_TRUNC, 0777);
    dup2(errorFd, STDERR_FILENO);
    close(errorFd);
  }
}

/**
 * @brief 子プロセス側でコマンドを execve により実行する
 * @param info       起動情報 (commandLine, environment, disableOutputCapture)
 * @param outputPath 標準出力キャプチャ先ファイルパス
 * @param errorPath  標準エラーキャプチャ先ファイルパス
 * @note disableOutputCapture が false の場合は CaptureOutput でリダイレクトしてから execve を呼ぶ。
 *       execve が成功した場合この関数は返らない。
 */
static void Execute(const ProcessStartInfo *info, const char *outputPath,
                    const char *errorPath) {
  const char *newArgv[] = {SHELL_NAME, OPTION_COMMAND, info->commandLine, NULL};
  if (!info->disableOutputCapture) {
    CaptureOutput(outputPath, errorPath);
  }
  execve(SHELL_PATH, (char *const *)newArgv, info->environment);
}

/**
 * @brief プロセスモジュールの静的リソースを初期化する
 * @details
 * 初回呼び出し時のみ実行される。ルートディレクトリの作成と
 * ProcessManager の初期化を行う。
 * @note スレッドセーフではない。プロセス起動前に一度だけ呼ぶこと。
 */
static void InitStatic() {
  if (!initialized) {
    initialized = true;
    CreateRootDirectory();
    ProcessManager_Init(CommonExitedHandler);
  }
}

/**
 * @brief timeval 構造体を秒単位の double 値に変換する
 * @param time 変換元の timeval 構造体
 * @return 秒単位の経過時間 (tv_sec + tv_usec をミリ秒精度で変換)
 */
static inline double TimevalToDouble(const struct timeval *time) {
  return (double)time->tv_sec +
         ((double)((long)(time->tv_usec / 1000)) / 1000.0);
}

/**
 * @brief 子プロセス側のエントリポイント
 * @param self Process インスタンス (子プロセス側のメモリ空間にコピーされたもの)
 * @details
 * ワークスペース上に出力キャプチャ用のファイルパスを構築し、
 * Execute() を呼び出して execve に移行する。Execute() が成功した場合は返らない。
 */
static void ChildProcess(Process *self) {
  char outputPath[64] = {0};
  sprintf(outputPath, "%s/" STANDARD_OUTPUT_FILE, self->workspace);
  char errorPath[64] = {0};
  sprintf(errorPath, "%s/" STANDARD_ERROR_FILE, self->workspace);
  Execute(&self->info, outputPath, errorPath); // no return
}
//! @}

/**
 * @brief コンストラクタ
 * @param self Process インスタンス
 * @param info 起動情報。NULL の場合は後で Process_Start() を呼び出す前に設定すること
 */
void Process_Init(Process *self, const ProcessStartInfo *info) {
  InitStatic();

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
  pthread_condattr_setclock(&conditionAttribute, CLOCK_REALTIME);
  pthread_cond_init(&self->condition, &conditionAttribute);
  pthread_condattr_destroy(&conditionAttribute);
}

/**
 * @brief Process を初期化して即座に起動する
 *
 * Process_Init() と Process_Start() を連続して呼び出す短縮形。
 * @param self Process インスタンス
 * @param info 起動情報
 */
void Process_Execute(Process *self, const ProcessStartInfo *info) {
  if (UNLIKELY(!self || !info || !info->commandLine)) {
    return;
  }
  Process_Init(self, info);
  Process_Start(self);
}

/**
 * @brief デストラクタ
 *
 * ワークスペースディレクトリと一時ファイルを削除し、
 * mutex・条件変数を解放する。
 * @param self Process インスタンス
 */
void Process_Destroy(Process *self) {
  pthread_cond_destroy(&self->condition);
  pthread_mutex_destroy(&self->mutex);
  RemoveWorkspace(self);
  CLEAR(self);
}

//! @name properties
//! @{

/**
 * @brief プロセスが正常終了したかどうかを返す
 * @param self Process インスタンス
 * @return プロセスが正常終了していれば true、そうでなければ false
 */
bool Process_HasExited(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return false;
  }
  if (!self->terminated) {
    return false;
  }
  return WIFEXITED(self->status);
}

/**
 * @brief プロセスの終了コードを返す
 * @param self Process インスタンス
 * @return 終了コード。未終了または無効な場合は -1
 */
int32_t Process_ExitCode(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  if (!self->terminated) {
    return -1;
  }
  return WEXITSTATUS(self->status);
}

/**
 * @brief プロセスが使用した CPU 時間の合計 (ユーザー + カーネル) を秒単位で返す
 * @param self Process インスタンス
 * @return 合計 CPU 時間 (秒)。未終了または無効な場合は -1
 */
double Process_TotalProcessorTime(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  if (!self->terminated) {
    return -1;
  }
  return TimevalToDouble(&self->usage.ru_utime) +
         TimevalToDouble(&self->usage.ru_stime);
}

/**
 * @brief プロセスが使用したユーザー空間 CPU 時間を秒単位で返す
 * @param self Process インスタンス
 * @return ユーザー CPU 時間 (秒)。未終了または無効な場合は -1
 */
double Process_UserProcessorTime(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  if (!self->terminated) {
    return -1;
  }
  return TimevalToDouble(&self->usage.ru_utime);
}

/**
 * @brief プロセスの開始時刻をエポック秒 (CLOCK_REALTIME ベース) で返す
 * @param self Process インスタンス
 * @return 開始時刻 (秒)。無効な場合は -1
 */
double Process_StartTime(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  return self->startTime;
}

/**
 * @brief プロセスの終了時刻をエポック秒 (CLOCK_REALTIME ベース) で返す
 * @param self Process インスタンス
 * @return 終了時刻 (秒)。未終了または無効な場合は -1
 */
double Process_ExitTime(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  return self->exitTime;
}

/**
 * @brief プロセス ID を返す
 * @param self Process インスタンス
 * @return プロセス ID。無効な場合は -1
 */
pid_t Process_Id(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return -1;
  }
  return self->id;
}

/**
 * @brief 標準出力を ProcessOutput に格納して返す
 * @param self Process インスタンス
 * @param[out] result 標準出力を格納する ProcessOutput
 * @note プロセスが終了していない場合や出力キャプチャが無効な場合は何もしない
 */
void Process_StandardOutput(Process *self, ProcessOutput *result) {
  if (UNLIKELY(!self || !self->id)) {
    return;
  }
  if (self->info.disableOutputCapture) {
    return;
  }
  if (!self->terminated) {
    return;
  }
  char outputPath[64] = {0};
  snprintf(outputPath, sizeof(outputPath), "%s/" STANDARD_OUTPUT_FILE,
           self->workspace);
  ProcessOutput_Init(result, outputPath);
  return;
}

/**
 * @brief 標準エラー出力を ProcessOutput に格納して返す
 * @param self Process インスタンス
 * @param[out] result 標準エラーを格納する ProcessOutput
 * @note プロセスが終了していない場合や出力キャプチャが無効な場合は何もしない
 */
void Process_StandardError(Process *self, ProcessOutput *result) {
  if (UNLIKELY(!self || !self->id)) {
    return;
  }
  if (self->info.disableOutputCapture) {
    return;
  }
  if (!self->terminated) {
    return;
  }
  char errorPath[64] = {0};
  snprintf(errorPath, sizeof(errorPath), "%s/" STANDARD_ERROR_FILE,
           self->workspace);
  ProcessOutput_Init(result, errorPath);
  return;
}
//! @}

//! @name public methods
//! @{

/**
 * @brief プロセスを非同期で起動する
 *
 * fork() で子プロセスを生成し、ProcessManager に登録する。
 * 終了時には onExited コールバックが呼ばれる。
 * @param self Process インスタンス
 */
void Process_Start(Process *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  self->id = fork();
  if (self->id < 0) {
    self->id = -1;
    return;
  }
  if (self->id == 0) {
    ChildProcess(self);
    _exit(PROCESS_FAILED); // unreachable
  }
  self->startTime = (double)GetRealTime() / 1000.0;
  ProcessManager_Register(self->id, self);
}

/**
 * @brief プロセスが終了するまでブロックして待機する
 * @param self Process インスタンス
 */
void Process_Wait(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
    return;
  }
  pthread_mutex_lock(&self->mutex);
  if (self->terminated) {
    pthread_mutex_unlock(&self->mutex);
    return;
  }
  self->terminated = true;
  pthread_mutex_unlock(&self->mutex);
  wait4(self->id, &self->status, 0, &self->usage);
}

/**
 * @brief 指定したタイムアウト時間内にプロセスが終了するのを待つ
 * @param self Process インスタンス
 * @param milliSeconds タイムアウト時間 (ミリ秒)
 * @return プロセスが終了した場合は true、タイムアウトした場合は false
 */
bool Process_WaitFor(Process *self, time_t milliSeconds) {
  if (UNLIKELY(!self || !self->id)) {
    return false;
  }

  time_t limit = GetMonotonicTime() + milliSeconds;
  while (!self->terminated) {
    if (GetMonotonicTime() > limit) {
      return false;
    }
    usleep(1000 * 1);
  }
  return true;
}

/**
 * @brief プロセスのサマリーを標準出力に表示する
 *
 * プロセス ID、終了コード、合計 CPU 時間、ユーザー CPU 時間、経過時間を出力する。
 * @param self Process インスタンス
 */
void Process_PrintSummary(Process *self) {
  if (UNLIKELY(!self || !self->id)) {
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
