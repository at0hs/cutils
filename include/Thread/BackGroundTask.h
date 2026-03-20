/**
 * @file BackGroundTask.h
 * @brief バックグラウンドタスク
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
#include <atomic>
#define ATOMIC(type) std::atomic<type>
extern "C" {
#else
#define ATOMIC(type) _Atomic type
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 制御ブロック
 */
struct BackGroundTask_t;
typedef struct BackGroundTask_t BackGroundTask_t;

/**
 * @brief 進捗状況
 */
typedef uint8_t BackGroundTaskProgress;

/**
 * @brief タスクのステータス
 */
typedef enum BackGroundTaskStatus {
  //! 失敗
  BACK_GROUND_TASK_FAILURE = -1,
  //! 初期
  BACK_GROUND_TASK_NONE,
  //! 正常終了
  BACK_GROUND_TASK_EXITED,
  //! 処理キャンセル
  BACK_GROUND_TASK_CANCELED,
  //! 強制中断
  BACK_GROUND_TASK_ABORTED,
} BackGroundTaskStatus;

/**
 * @brief イベントハンドラ
 */
typedef struct BackGroundTaskEventHandlers_t {
  //! セットアップ
  void (*setup)(BackGroundTask_t *self);
  //! 進捗更新
  void (*progressChanged)(BackGroundTask_t *self,
                          BackGroundTaskProgress newProgress);
  //! タスク終了
  void (*finished)(BackGroundTask_t *self, BackGroundTaskStatus exitStatus);
} BackGroundTaskEventHandlers_t;

/**
 * @brief タスク
 */
typedef struct BackGroundTaskWork_t {
  BackGroundTaskStatus (*func)(BackGroundTask_t *self, void *sender);
  void *sender;
} BackGroundTaskWork_t;

/**
 * @brief 制御ブロック
 */
struct BackGroundTask_t {
  //! @name Private
  //! @{

  //! スレッド ID
  pthread_t id;
  //! タスク実行中フラグ
  ATOMIC(bool) isBusy;
  //! スレッド終了フラグ
  ATOMIC(bool) exited;
  //! 実行するワーク
  BackGroundTaskWork_t work;

  //! @}

  //! @name Public
  //! @{

  //! スレッド名（最大 15 文字 + NUL）
  char name[16];
  //! イベントハンドラ群
  BackGroundTaskEventHandlers_t eventHandlers;
  //! 現在の進捗（0〜255）
  ATOMIC(BackGroundTaskProgress) progress;
  //! タスクの終了ステータス
  ATOMIC(BackGroundTaskStatus) exitStatus;
  //! キャンセル要求フラグ。ワーク関数内で定期的に確認すること
  ATOMIC(bool) cancellationPending;

  //! @}
};

/**
 * @brief バックグラウンドタスクを初期化する
 * @param self インスタンス
 * @param work 実行するワーク（func と sender を設定しておくこと）
 */
void BackGroundTask_Init(BackGroundTask_t *self, BackGroundTaskWork_t *work);

/**
 * @brief バックグラウンドタスクをスレッドで開始する
 * @param self インスタンス
 * @return 0: 成功, -1: 失敗（既に実行中または pthread_create エラー）
 */
int BackGroundTask_Run(BackGroundTask_t *self);

/**
 * @brief 進捗を更新して progressChanged ハンドラを呼び出す
 * @param self インスタンス
 * @param newProgress 新しい進捗値（0〜255）
 */
void BackGroundTask_ReportsProgress(BackGroundTask_t *self,
                                    BackGroundTaskProgress newProgress);

/**
 * @brief タスクが実行中か判定する
 * @param self インスタンス
 * @return true: 実行中, false: 停止中または self が NULL
 */
bool BackGroundTask_IsRunning(BackGroundTask_t *self);

/**
 * @brief タスクを強制中断する（pthread_cancel を使用）
 * @note finished ハンドラは BACK_GROUND_TASK_ABORTED で呼び出される
 * @param self インスタンス
 */
void BackGroundTask_Abort(BackGroundTask_t *self);

/**
 * @brief キャンセル要求を送る（cancellationPending を true にする）
 * @note 実際の中断はワーク関数内で cancellationPending を確認して行うこと
 * @param self インスタンス
 */
void BackGroundTask_Cancel(BackGroundTask_t *self);

/**
 * @brief タスクを破棄する
 * @note 実行中の場合は Abort してスレッド終了を待ってからリソースを解放する
 * @param self インスタンス
 */
void BackGroundTask_Destroy(BackGroundTask_t *self);

#ifdef __cplusplus
}
#endif
