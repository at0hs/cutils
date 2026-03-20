/**
 * @file ProcessManager.h
 * @brief プロセスマネージャー (内部モジュール)
 *
 * SIGCHLD シグナルを監視し、子プロセスの終了を検知して
 * 登録済みのコールバックを呼び出すシングルトン。
 * @author atohs
 * @date 2024/08/29
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>

/**
 * @brief 子プロセス終了時に呼ばれるコールバック型
 * @param pid 終了した子プロセスの PID
 * @param exitStatus wait4() が返すステータス値
 * @param data ProcessManager_Register() で登録したユーザーデータ
 */
typedef void (*ProcessExitedEventHandler)(pid_t pid, int32_t exitStatus,
                                          void *data);

/**
 * @brief ProcessManager を初期化する
 *
 * SIGCHLD ハンドラを設定し、プロセステーブルを確保する。
 * 2 回目以降の呼び出しは無視される。
 * @param onExited 子プロセス終了時に呼び出すコールバック
 */
void ProcessManager_Init(ProcessExitedEventHandler onExited);

/**
 * @brief ProcessManager を破棄する
 *
 * プロセステーブルを解放する。
 */
void ProcessManager_Destroy();

/**
 * @brief プロセスをテーブルに登録する
 * @param pid 監視対象プロセスの PID
 * @param data 終了時コールバックに渡すユーザーデータ
 */
void ProcessManager_Register(pid_t pid, void *data);

/**
 * @brief プロセスをテーブルから登録解除する
 * @param pid 登録解除するプロセスの PID
 */
void ProcessManager_Unregister(pid_t pid);

#ifdef __cplusplus
}
#endif
