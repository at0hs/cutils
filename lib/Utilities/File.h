/**
 * @file File.h
 * @brief ファイル操作ユーティリティ
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <unistd.h>

/**
 * @brief ファイルサイズを取得する
 * @param path ファイルパス
 * @return 成功時はファイルサイズ (バイト)、失敗時は `-errno` (負値)
 */
ssize_t File_GetSize(const char *path);

/**
 * @brief ファイルが存在するか確認する
 * @param path ファイルパス
 * @return 存在する場合 `true`、存在しない場合 `false`
 */
bool File_Exists(const char *path);

/**
 * @brief ファイルを読み込んでバッファに格納する
 * @param path   読み込むファイルパス
 * @param buffer 読み込みデータの格納先バッファ
 * @param size   読み込むバイト数
 * @return 成功時は `0`、失敗時は `-errno` (負値)
 */
int File_Read(const char *path, void *buffer, size_t size);

/**
 * @brief 空のファイルを新規作成する
 * @param path 作成するファイルパス
 * @return 成功時は `0`、失敗時は `-errno` (負値)
 * @note ファイルが既に存在する場合は内容が切り捨てられる
 */
int File_Create(const char *path);

#ifdef __cplusplus
}
#endif
