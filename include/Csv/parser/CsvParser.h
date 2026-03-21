/**
 * @file CsvParser.h
 * @brief Csvパーサー
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../Csv.h"
#include "../content/CsvContent.h"
#include "../property/CsvProperty.h"
#include <stdlib.h>

/**
 * @brief 制御ブロック
 */
struct CsvParser_t;
typedef struct CsvParser_t CsvParser_t;

/**
 * @brief CSVパーサーを初期化する。
 * @param[in] props CSVプロパティへのポインタ
 * @return 初期化されたパーサーへのポインタ。失敗時は NULL
 */
extern CsvParser_t *CsvParser_Init(const CsvProperties_t *props);

/**
 * @brief ファイルからCSVを読み込み、解析する。
 * @param[in,out] self     CsvParser_t 構造体へのポインタ
 * @param[in]     filePath 読み込むファイルのパス
 * @return 処理結果 (@ref CsvReturnCode)
 */
extern CsvReturnCode CsvParser_LoadFromFile(CsvParser_t *self,
                                            const char *filePath);

/**
 * @brief メモリ上のデータからCSVを読み込み、解析する。
 * @param[in,out] self     CsvParser_t 構造体へのポインタ
 * @param[in]     data     CSVデータへのポインタ
 * @param[in]     dataSize データのバイトサイズ
 * @return 処理結果 (@ref CsvReturnCode)
 */
extern CsvReturnCode CsvParser_LoadFromData(CsvParser_t *self,
                                            const char *data,
                                            size_t dataSize);

/**
 * @brief 解析済みのコンテンツを取得する。
 * @param[in] self CsvParser_t 構造体へのポインタ
 * @return 解析結果の CsvContent_t へのポインタ。失敗時は NULL
 */
extern const CsvContent_t *CsvParser_GetContent(CsvParser_t *self);

/**
 * @brief CSVパーサーを破棄し、リソースを解放する。
 * @param[in,out] self CsvParser_t 構造体へのポインタ
 */
extern void CsvParser_Destroy(CsvParser_t *self);

#ifdef __cplusplus
}
#endif
