/**
 * @file Csv.h
 * @brief
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ステータス
 */
typedef enum CsvReturnCode {
  CSV_SUCCESS = 0,        //!< 成功
  CSV_INVALID_PARAMETER,  //!< 無効な引数
  CSV_INVALID_CHAR_CODE,  //!< サポート外の文字コードが含まれている
  CSV_OUT_OF_MEMORY,  //!< メモリ確保に失敗した
} CsvReturnCode;

#ifdef __cplusplus
}
#endif
