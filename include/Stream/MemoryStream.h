/**
 * @file MemoryStream.h
 * @brief インメモリバイトストリーム
 * @author atohs
 * @date 2024/11/05
 *
 * @details
 * `open_memstream` をバックエンドとしたインメモリの読み書きストリームを提供します。
 * Read/Write/Seek の各操作をサポートし、書き込んだデータはバッファ経由で参照できます。
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>

/**
 * @brief MemoryStream 操作の戻り値コード
 */
typedef enum MemoryStreamReturnCode {
  MEMORY_STREAM_SUCCESS = 0,      /**< 操作成功 */
  MEMORY_STREAM_INVALID_ARG = -1, /**< 引数が NULL または不正 */
  MEMORY_STREAM_OPEN_FAILED = -2, /**< open_memstream の失敗 */
  MEMORY_STREAM_SEEK_ERROR = -3,  /**< シーク操作の失敗 */
} MemoryStreamReturnCode;

/**
 * @brief インメモリバイトストリームの管理構造体
 *
 * @details
 * `open_memstream` で生成した FILE ストリームと、それに紐づくバッファを保持します。
 * `size` は書き込みの高水位標（最大書き込みオフセット）を示し、論理データサイズとして
 * 読み取り範囲の制御に使用されます。
 */
typedef struct MemoryStream {
  FILE *stream; /**< open_memstream で生成した FILE ポインタ */
  char *buffer; /**< open_memstream が管理するバッファ; fflush 後に有効 */
  size_t
      position; /**< sizeloc: fflush 後に open_memstream が書き込む現在の書き込み位置 */
  size_t size; /**< 高水位標: これまでの最大書き込み位置（論理データサイズ） */
} MemoryStream;

/**
 * @brief MemoryStream を初期化する
 *
 * @param[out] self 初期化する MemoryStream へのポインタ
 * @return MEMORY_STREAM_SUCCESS         初期化成功
 * @return MEMORY_STREAM_INVALID_ARG    `self` が NULL
 * @return MEMORY_STREAM_OPEN_FAILED    `open_memstream` が失敗
 */
MemoryStreamReturnCode MemoryStream_Init(MemoryStream *self);

/**
 * @brief MemoryStream を破棄してリソースを解放する
 *
 * @details
 * ストリームをクローズし、内部バッファを解放します。
 * `self` が NULL の場合は何もしません。
 *
 * @param[in,out] self 破棄する MemoryStream へのポインタ
 */
void MemoryStream_Destroy(MemoryStream *self);

/**
 * @brief ストリームの現在位置にデータを書き込む
 *
 * @param[in,out] self  書き込み先の MemoryStream
 * @param[in]     data  書き込むデータへのポインタ
 * @param[in]     size  書き込むバイト数
 * @return 書き込んだバイト数 (`size` と等しい)。失敗時は -1。
 */
ssize_t MemoryStream_Write(MemoryStream *self, const void *data, size_t size);

/**
 * @brief ストリームの現在位置からデータを読み取る
 *
 * @details
 * 現在のシーク位置から最大 `size` バイトを `buffer` にコピーします。
 * 読み取り後、シーク位置は読み取ったバイト数だけ進みます。
 * 現在位置が高水位標 (`size`) に達している場合は 0 を返します。
 *
 * @param[in,out] self    読み取り元の MemoryStream
 * @param[out]    buffer  読み取ったデータを格納するバッファ
 * @param[in]     size    読み取る最大バイト数
 * @return 実際に読み取ったバイト数。読み取れるデータがない場合は 0。
 */
size_t MemoryStream_Read(MemoryStream *self, void *buffer, size_t size);

/**
 * @brief ストリームのシーク位置を変更する
 *
 * @param[in,out] self    対象の MemoryStream
 * @param[in]     offset  基準位置からのオフセット (バイト単位)
 * @param[in]     whence  基準位置 (`SEEK_SET`, `SEEK_CUR`, `SEEK_END`)
 * @return MEMORY_STREAM_SUCCESS      シーク成功
 * @return MEMORY_STREAM_INVALID_ARG `self` が NULL
 * @return MEMORY_STREAM_SEEK_ERROR  `fseeko` が失敗
 */
MemoryStreamReturnCode MemoryStream_Seek(MemoryStream *self,
                                         off_t offset,
                                         int whence);

/**
 * @brief 内部バッファへの読み取り専用ポインタを返す
 *
 * @details
 * 返されたポインタは次の Write 操作や Destroy 呼び出しで無効になる場合があります。
 * バッファの内容を保持したい場合はコピーしてください。
 *
 * @param[in,out] self 対象の MemoryStream (`fflush` が内部で呼ばれます)
 * @return 内部バッファへのポインタ。`self` が NULL の場合は NULL。
 */
const void *MemoryStream_RefBuffer(MemoryStream *self);

/**
 * @brief これまでの最大書き込みサイズ（論理データサイズ）を返す
 *
 * @param[in] self 対象の MemoryStream
 * @return 論理データサイズ (バイト単位)。`self` が NULL の場合は 0。
 */
size_t MemoryStream_GetSize(MemoryStream *self);

#ifdef __cplusplus
}
#endif
