/**
 * @file CsvContent.h
 * @brief コンテンツ
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../content/line/CsvLineCollection.h"
#include <stdbool.h>

/**
 * @brief 制御ブロック
 */
typedef struct CsvContent_t {
  //! @name Private
  //! @{

  CsvLineCollection_t lines;  //!< 行のリスト

  //! @}
} CsvContent_t;

/**
 * @brief CSVコンテンツを初期化する。
 * @param[in,out] self CsvContent_t 構造体へのポインタ
 */
extern void CsvContent_Init(CsvContent_t *self);

/**
 * @brief 行の所有権を移動し、コンテンツの末尾に追加する。
 * @param[in,out] self CsvContent_t 構造体へのポインタ
 * @param[in,out] line 追加する CsvLine_t 構造体へのポインタ（所有権が移動する）
 * @return 追加できた場合は true。メモリ確保に失敗した場合は false（引数側は破棄される）
 */
extern bool CsvContent_MoveBackLine(CsvContent_t *self, CsvLine_t *line);

/**
 * @brief CSVコンテンツを破棄し、リソースを解放する。
 * @param[in,out] self CsvContent_t 構造体へのポインタ
 */
extern void CsvContent_Destroy(CsvContent_t *self);

/**
 * @brief CSVコンテンツの内容を標準出力に表示する。
 * @param[in] self CsvContent_t 構造体へのポインタ
 */
extern void CsvContent_Print(const CsvContent_t *self);

/**
 * @brief 2つのCSVコンテンツが等しいか判定する。
 * @param[in] self    比較元の CsvContent_t 構造体へのポインタ
 * @param[in] content 比較対象の CsvContent_t 構造体へのポインタ
 * @return 等しい場合は true、それ以外は false
 */
extern bool CsvContent_Equals(const CsvContent_t *self,
                              const CsvContent_t *content);

#ifdef __cplusplus
}
#endif
