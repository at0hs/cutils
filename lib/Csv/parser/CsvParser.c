/**
 * @file CsvParser.c
 * @brief CSVパーサー
 * @author atohs
 * @date 2024/07/12
 */
#include "Csv/parser/CsvParser.h"
#include "Csv/content/CsvContent.h"
#include "Csv/property/CsvProperty.h"
#include "utilities.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief ファイルバッファ
 */
typedef struct FileBuffer_t {
  char *data;
  size_t size;
} FileBuffer_t;

/**
 * @struct CsvParser_t
 * @brief CSVパーサー構造体
 */
struct CsvParser_t {
  CsvProperties_t properties;  //! プロパティ
  CsvContent_t content;        //! 解析後のデータ
  FileBuffer_t file;           //! 元のファイルデータ
};

//! @brief バッファ番兵バイト(0xFF は ASCII 範囲外のため、ASCII
//! 専用データ中には出現しない)
#define CSV_EOF_BYTE ((uint8_t)0xFF)
//! @brief 番兵バイトの長さ
#define EOF_LENGTH (sizeof(CSV_EOF_BYTE))
//! @brief コンマ
#define COMMA (',')
//! @brief キャリッジリターン
#define CARRIGE_RETURN ('\r')
//! @brief ラインフィード
#define LINE_FEED ('\n')

/**
 * @brief ファイルの終端か判定する。
 * @param[in] c 判定する文字
 * @return 終端であればtrue、そうでなければfalse
 */
static inline bool IsEof(char c) {
  return (uint8_t)c == CSV_EOF_BYTE;
}

/**
 * @brief コンマか判定する。
 * @param[in] c 判定する文字
 * @return コンマであればtrue、そうでなければfalse
 */
static inline bool IsComma(char c) {
  return c == COMMA;
}

/**
 * @brief キャリッジリターン(\r)か判定する。
 * @param[in] c 判定する文字
 * @return キャリッジリターンであればtrue、そうでなければfalse
 */
static inline bool IsCr(char c) {
  return c == CARRIGE_RETURN;
}

/**
 * @brief ラインフィード(\n)か判定する。
 * @param[in] c 判定する文字
 * @return ラインフィードであればtrue、そうでなければfalse
 */
static inline bool IsLf(char c) {
  return c == LINE_FEED;
}

/**
 * @brief CRLF(\r\n)か判定する。
 * @param[in] c        判定する文字
 * @param[in] nextChar 次の文字
 * @return CRLFであればtrue、そうでなければfalse
 */
static inline bool IsCrLf(char c, char nextChar) {
  return IsCr(c) && IsLf(nextChar);
}

/**
 * @brief 文字を終端(NULL文字)にする。
 * @param[out] c 終端させる文字へのポインタ
 */
static inline void Terminate(char *c) {
  *c = '\0';
}

/**
 * @brief 不要な文字(NUL / CR / LF)か判定する。
 * @param[in] c 判定する文字
 * @return 不要な文字であればtrue、そうでなければfalse
 */
static inline bool IsExtraChars(char c) {
  return c == '\0' || IsLf(c) || IsCr(c);
}

/**
 * @brief ASCII文字か判定する。
 * @param[in] c 判定する文字
 * @return ASCII文字であればtrue、そうでなければfalse
 */
static inline bool IsAscii(char c) {
  return (uint8_t)c <= 0x7F;
}

/**
 * @brief サポートされていない文字コードが含まれているか判定する。
 * @param[in] data データ
 * @param[in] size サイズ
 * @return サポート外の文字が含まれていればtrue、すべてサポート内であればfalse
 */
static bool IsNotSupportedCharCode(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (!IsAscii(data[i]) && !IsEof(data[i])) {
      return true;
    }
  }
  return false;
}

/**
 * @brief バッファの末尾をEOFで埋める。
 * @param[in,out] data データバッファ
 * @param[in]     size バッファサイズ
 */
static void FillEof(char *data, size_t size) {
  // 最後の値は必ずEofにする
  data[size - 1] = CSV_EOF_BYTE;
  // size_t のアンダーフローを利用して index 0 まで処理する
  // i-- で 0 → SIZE_MAX にラップし、SIZE_MAX < size - 1 が偽になってループ終了
  for (size_t i = size - 2; i < size - 1; i--) {
    if (!IsExtraChars(data[i])) {
      return;
    }
    data[i] = CSV_EOF_BYTE;
  }
}

/**
 * @brief CSVデータを解析する。
 * @param[in,out] self インスタンス
 * @return 処理結果
 */
static CsvReturnCode Parse(CsvParser_t *self) {
  if (IsNotSupportedCharCode(self->file.data, self->file.size)) {
    return CSV_INVALID_CHAR_CODE;
  }
  CsvLine_t line;
  CsvItem_t item;
  CsvLine_Init(&line);
  CsvItem_Init(&item);
  CsvItem_Set(&item, self->file.data);

  for (char *c = self->file.data; true; c++) {
    if (IsEof(*c)) {
      Terminate(c);
      CsvLine_MoveBackItem(&line, &item);
      CsvContent_MoveBackLine(&self->content, &line);
      break;
    }
    char *nextChar = c + 1;
    if (IsComma(*c)) {
      Terminate(c);  // 終端
      CsvLine_MoveBackItem(&line, &item);
      CsvItem_Set(&item, nextChar);
    } else if (IsCrLf(*c, *nextChar)) {
      Terminate(c);
      Terminate(nextChar);
      CsvLine_MoveBackItem(&line, &item);
      CsvContent_MoveBackLine(&self->content, &line);
      CsvLine_Init(&line);
      CsvItem_Set(&item, nextChar + 1);
    } else if (IsCr(*c)) {
      Terminate(c);
      CsvLine_MoveBackItem(&line, &item);
      CsvContent_MoveBackLine(&self->content, &line);
      CsvLine_Init(&line);
      CsvItem_Set(&item, nextChar);
    } else if (IsLf(*c)) {
      Terminate(c);
      CsvLine_MoveBackItem(&line, &item);
      CsvContent_MoveBackLine(&self->content, &line);
      CsvLine_Init(&line);
      CsvItem_Set(&item, nextChar);
    } else {
      // do nothing
    }
  }
  return CSV_SUCCESS;
}

/**
 * @brief ファイルを読み込む。
 * @param[in,out] self     インスタンス
 * @param[in]     filePath ファイルパス
 * @return 成功時は0、失敗時は負のエラーコード
 */
static int LoadFile(CsvParser_t *self, const char *filePath) {
  ssize_t file_size = File_GetSize(filePath);
  if (file_size < 0) {
    return (int)file_size;
  }
  size_t allocate_size = file_size + EOF_LENGTH;
  self->file.data = malloc(allocate_size);
  if (!self->file.data) {
    return -1;
  }
  int ret = File_Read(filePath, self->file.data, file_size);
  if (ret < 0) {
    free(self->file.data);
    self->file.data = NULL;
    return ret;
  }
  self->file.size = allocate_size;
  FillEof(self->file.data, self->file.size);
  return 0;
}

/**
 * @brief ロード前にインスタンスをリセットする。
 *
 * 2回目以降のLoad呼び出し時に前回のバッファとコンテンツを解放する。
 * @param[in,out] self インスタンス
 */
static void Reset(CsvParser_t *self) {
  if (self->file.data) {
    free(self->file.data);
    self->file.data = NULL;
    self->file.size = 0;
  }
  CsvContent_Destroy(&self->content);
  CsvContent_Init(&self->content);
}

/**
 * @brief CSVパーサーを初期化する。
 * @param[in] props CSVプロパティへのポインタ
 * @return 初期化されたパーサーへのポインタ。失敗時はNULL
 */
CsvParser_t *CsvParser_Init(const CsvProperties_t *props) {
  if (UNLIKELY(!props)) {
    return NULL;
  }
  CsvParser_t *self = (CsvParser_t *)calloc(1, sizeof(*self));
  if (UNLIKELY(!self)) {
    return NULL;
  }
  self->properties = *props;
  CsvContent_Init(&self->content);
  return self;
}

/**
 * @brief ファイルからCSVを読み込む。
 * @param[in,out] self     CsvParser_t構造体へのポインタ
 * @param[in]     filePath ファイルパス
 * @return 処理結果
 */
CsvReturnCode CsvParser_LoadFromFile(CsvParser_t *self, const char *filePath) {
  if (UNLIKELY(!self || !filePath)) {
    return CSV_INVALID_PARAMETER;
  }
  Reset(self);
  if (LoadFile(self, filePath) != 0) {
    return CSV_INVALID_PARAMETER;
  }
  return Parse(self);
}

/**
 * @brief メモリ上のデータからCSVを読み込む。
 * @param[in,out] self     CsvParser_t構造体へのポインタ
 * @param[in]     data     CSVデータへのポインタ
 * @param[in]     dataSize データのサイズ
 * @return 処理結果
 */
CsvReturnCode CsvParser_LoadFromData(CsvParser_t *self,
                                     const char *data,
                                     size_t dataSize) {
  if (UNLIKELY(!self || !data || dataSize == 0)) {
    return CSV_INVALID_PARAMETER;
  }
  Reset(self);
  size_t allocate_size = dataSize + EOF_LENGTH;
  self->file.data = malloc(allocate_size);
  if (UNLIKELY(!self->file.data)) {
    return CSV_INVALID_PARAMETER;
  }
  self->file.size = allocate_size;
  memcpy(self->file.data, data, dataSize);
  FillEof(self->file.data, allocate_size);
  return Parse(self);
}

/**
 * @brief 解析されたコンテンツを取得する。
 * @param[in] self CsvParser_t構造体へのポインタ
 * @return 解析結果のコンテンツへのポインタ。失敗時はNULL
 */
const CsvContent_t *CsvParser_GetContent(CsvParser_t *self) {
  if (UNLIKELY(!self)) {
    return NULL;
  }
  return &self->content;
}

/**
 * @brief CSVパーサーを破棄する。
 * @param[in,out] self CsvParser_t構造体へのポインタ
 */
void CsvParser_Destroy(CsvParser_t *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  CsvContent_Destroy(&self->content);
  if (self->file.data)
    free(self->file.data);
  free(self);
}
