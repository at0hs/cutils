/**
 * @file CsvProperty.h
 * @brief CSVパーサーのプロパティ定義
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#include <stdbool.h>

/**
 * @brief 改行コード
 */
typedef enum CsvEolCode {
  CSV_EOL_CR = 0,  //!< CR (\r) のみ
  CSV_EOL_CRLF,    //!< CRLF (\r\n)
} CsvEolCode;

/**
 * @brief CSVパーサーのプロパティ
 */
typedef struct CsvProperties {
  bool hasHeader;  //!< ヘッダ行を持つCSVかどうか
} CsvProperties_t;
