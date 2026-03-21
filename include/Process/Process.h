/**
 * @file Process.h
 * @brief プロセス
 * @author atohs
 * @date 2024/08/29
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "Process/ProcessOutput.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

/**
 * @brief Information to start a process
 */
typedef struct ProcessStartInfo {
  /**
   * @brief Command line to execute
   */
  const char *commandLine;
  /**
   * @brief Environment variables
   */
  char *const *environment;

  /**
   * @brief 標準出力と標準エラーのキャプチャを無効にするフラグ
   *
   * true の場合、子プロセスの標準出力・標準エラーはファイルにリダイレクトされず、
   * 親プロセスの端末にそのまま出力される。
   */
  bool disableOutputCapture;
} ProcessStartInfo;

/**
 * @brief A process
 */
typedef struct Process {
  //! @name private
  //! @{

  ProcessStartInfo info;  //! プロセス起動情報
  int32_t status;  //! プロセスの終了ステータス（WIFEXITED, WIFSIGNALED など）
  pthread_mutex_t mutex;     //! スレッド同期用の mutex
  pthread_cond_t condition;  //! スレッド同期用の条件変数
  bool isWaiting;            //! プロセス終了待機中かどうか
  struct rusage usage;       //! プロセスのリソース使用量
  char workspace[32];        //! ワークスペースディレクトリパス
  pid_t id;                  //! プロセス ID
  struct {
    void *address;   //! スタックのメモリアドレス
    size_t size;     //! スタックのサイズ
  } stack;           //! プロセスのスタック領域情報
  double startTime;  //! プロセス開始時刻（エポック秒、CLOCK_REALTIME ベース）
  double exitTime;   //! プロセス終了時刻（エポック秒、CLOCK_REALTIME ベース）
  bool terminated;   //! プロセスが終了したかどうか

  //! @}

  //! @name public
  //! @{

  void (*onExited)(
      struct Process
          *self);  //! プロセス終了時に呼ばれるコールバック関数ポインタ

  //! @}
} Process;

/**
 * @brief Process を初期化する (コンストラクタ)
 * @param self Process インスタンス
 * @param info 起動情報。NULL の場合は後で Process_Start を呼び出す前に設定すること
 */
extern void Process_Init(Process *self, const ProcessStartInfo *info);

/**
 * @brief Process を破棄する (デストラクタ)
 *
 * ワークスペースディレクトリと一時ファイルを削除し、
 * mutex・条件変数を解放する。
 * @param self Process インスタンス
 */
extern void Process_Destroy(Process *self);

/**
 * @brief Process を初期化して即座に起動する
 *
 * Process_Init() と Process_Start() を連続して呼び出す短縮形。
 * @param self Process インスタンス
 * @param info 起動情報
 */
extern void Process_Execute(Process *self, const ProcessStartInfo *info);

/**
 * @brief プロセスが正常終了したかどうかを返す
 * @param self Process インスタンス
 * @return プロセスが終了していれば true、そうでなければ false
 */
extern bool Process_HasExited(Process *self);

/**
 * @brief プロセスの終了コードを返す
 * @param self Process インスタンス
 * @return 終了コード。未終了または無効な場合は -1
 */
extern int32_t Process_ExitCode(Process *self);

/**
 * @brief プロセスが使用した CPU 時間の合計 (ユーザー + カーネル) を秒単位で返す
 * @param self Process インスタンス
 * @return 合計 CPU 時間 (秒)。未終了または無効な場合は -1
 */
extern double Process_TotalProcessorTime(Process *self);

/**
 * @brief プロセスが使用したユーザー空間 CPU 時間を秒単位で返す
 * @param self Process インスタンス
 * @return ユーザー CPU 時間 (秒)。未終了または無効な場合は -1
 */
extern double Process_UserProcessorTime(Process *self);

/**
 * @brief プロセスの開始時刻をエポック秒 (CLOCK_REALTIME ベース) で返す
 * @param self Process インスタンス
 * @return 開始時刻 (秒)。無効な場合は -1
 */
extern double Process_StartTime(Process *self);

/**
 * @brief プロセスの終了時刻をエポック秒 (CLOCK_REALTIME ベース) で返す
 * @param self Process インスタンス
 * @return 終了時刻 (秒)。未終了または無効な場合は -1
 */
extern double Process_ExitTime(Process *self);

/**
 * @brief プロセス ID を返す
 * @param self Process インスタンス
 * @return プロセス ID。無効な場合は -1
 */
extern pid_t Process_Id(Process *self);

/**
 * @brief 標準出力を ProcessOutput に格納して返す
 * @param self Process インスタンス
 * @param[out] result 標準出力を格納する ProcessOutput
 * @note プロセスが終了していない場合や出力キャプチャが無効な場合は何もしない
 */
extern void Process_StandardOutput(Process *self, ProcessOutput *result);

/**
 * @brief 標準エラー出力を ProcessOutput に格納して返す
 * @param self Process インスタンス
 * @param[out] result 標準エラーを格納する ProcessOutput
 * @note プロセスが終了していない場合や出力キャプチャが無効な場合は何もしない
 */
extern void Process_StandardError(Process *self, ProcessOutput *result);

/**
 * @brief プロセスを非同期で起動する
 *
 * fork() で子プロセスを生成し、ProcessManager に登録する。
 * 終了時には onExited コールバックが呼ばれる。
 * @param self Process インスタンス
 */
extern void Process_Start(Process *self);

/**
 * @brief プロセスが終了するまでブロックして待機する
 * @param self Process インスタンス
 */
extern void Process_Wait(Process *self);

/**
 * @brief 指定したタイムアウト時間内にプロセスが終了するのを待つ
 * @param self Process インスタンス
 * @param milliSeconds タイムアウト時間 (ミリ秒)
 * @return プロセスが終了した場合は true、タイムアウトした場合は false
 */
extern bool Process_WaitFor(Process *self, time_t milliSeconds);

/**
 * @brief プロセスのサマリーを標準出力に表示する
 *
 * プロセス ID、終了コード、合計 CPU 時間、ユーザー CPU 時間、経過時間を出力する。
 * @param self Process インスタンス
 */
extern void Process_PrintSummary(Process *self);

#ifdef __cplusplus
}
#endif
