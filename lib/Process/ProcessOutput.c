/**
 * @file ProcessOutput.c
 * @brief プロセスの出力
 * @author atohs
 * @date 2024/09/01
 */
#include "Process/ProcessOutput.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief ProcessOutput を初期化する (コンストラクタ)
 * @param self ProcessOutput インスタンス
 * @param outputFile 出力が保存されているファイルのパス
 */
void ProcessOutput_Init(ProcessOutput *self, const char *outputFile) {
  if (UNLIKELY(!self || !outputFile)) {
    return;
  }
  CLEAR(self);
  strncpy(self->path, outputFile, sizeof(self->path) - 1);
  ssize_t size = File_GetSize(outputFile);
  self->size = size > 0 ? (size_t)size : 0;  // 取得失敗 (-errno) は空として扱う
}

/**
 * @brief ProcessOutput を破棄する (デストラクタ)
 * @param self ProcessOutput インスタンス
 */
void ProcessOutput_Destroy(ProcessOutput *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  CLEAR(self);
}

/**
 * @brief 出力内容を新規確保したバッファに読み込んで返す
 * @param self ProcessOutput インスタンス
 * @return 出力内容を格納したヒープバッファ (呼び出し側が free() する責任を持つ)。
 *         出力が空またはエラーの場合は NULL
 */
char *ProcessOutput_Read(ProcessOutput *self) {
  if (UNLIKELY(!self || !self->size)) {
    return NULL;
  }
  FILE *fp = fopen(self->path, "r");
  if (!fp) {
    return NULL;
  }
  char *buffer = calloc(1, self->size + 1);
  if (buffer) {
    if (fread(buffer, self->size, 1, fp) != 1) {
      free(buffer);
      fclose(fp);
      return NULL;
    }
  }
  fclose(fp);
  return buffer;
}

/**
 * @brief 出力内容を指定したバッファに読み込む
 * @param self ProcessOutput インスタンス
 * @param[out] buffer 読み込み先バッファ
 * @param bufferSize バッファのサイズ (バイト)
 * @return 実際に読み込んだバイト数。エラーまたは出力が空の場合は -1
 */
ssize_t ProcessOutput_ReadTo(ProcessOutput *self,
                             char *buffer,
                             size_t bufferSize) {
  if (UNLIKELY(!self || !self->size)) {
    return -1;
  }
  FILE *fp = fopen(self->path, "r");
  if (!fp) {
    return -1;
  }
  size_t size =
      fread(buffer, 1, bufferSize > self->size ? self->size : bufferSize, fp);
  fclose(fp);
  return (ssize_t)size;
}
