/**
 * @file ProcessOutput.h
 * @brief プロセスの出力
 * @author atohs
 * @date 2024/09/01
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * @brief プロセスの出力を保持する構造体
 *
 * 子プロセスの標準出力または標準エラーを一時ファイル経由で読み取るためのハンドル。
 */
typedef struct ProcessOutput {
  //! @name public
  //! @{

  size_t size;    //! 出力データのバイトサイズ
  char path[64];  //! 出力が保存された一時ファイルのパス

  //! @}
} ProcessOutput;

/**
 * @brief ProcessOutput を初期化する (コンストラクタ)
 * @param self ProcessOutput インスタンス
 * @param outputFile 出力が保存されているファイルのパス
 */
extern void ProcessOutput_Init(ProcessOutput *self, const char *outputFile);

/**
 * @brief ProcessOutput を破棄する (デストラクタ)
 * @param self ProcessOutput インスタンス
 */
extern void ProcessOutput_Destroy(ProcessOutput *self);

/**
 * @brief 出力内容を新規確保したバッファに読み込んで返す
 * @param self ProcessOutput インスタンス
 * @return 出力内容を格納したヒープバッファ (呼び出し側が free() する責任を持つ)。
 *         出力が空またはエラーの場合は NULL
 */
extern char *ProcessOutput_Read(ProcessOutput *self);

/**
 * @brief 出力内容を指定したバッファに読み込む
 * @param self ProcessOutput インスタンス
 * @param[out] buffer 読み込み先バッファ
 * @param bufferSize バッファのサイズ (バイト)
 * @return 実際に読み込んだバイト数。エラーまたは出力が空の場合は -1
 */
extern ssize_t ProcessOutput_ReadTo(ProcessOutput *self,
                                    char *buffer,
                                    size_t bufferSize);

#ifdef __cplusplus
}
#endif
