/**
 * @file Dictionary.h
 * @brief 辞書型
 * @author atohs
 * @date 2024/07/12
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <search.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 辞書のキーに対応するオブジェクト
 */
typedef struct DictionaryObject_t {
  //! バッファ
  void *buffer;
  //! サイズ
  size_t size;
} DictionaryObject_t;

/**
 * @brief 要素
 */
typedef struct DictionaryElement_t {
  //! キー
  char *key;
  //! オブジェクト
  DictionaryObject_t object;
} DictionaryElement_t;

/**
 * @brief 制御ブロック
 */
typedef struct Dictionary_t {
  //! @name Private
  //! @{

  //! ハッシュテーブル
  struct hsearch_data hashTable;
  /**
   * @brief 要素のリスト
   */
  struct {
    //! リストの実態
    DictionaryElement_t *list;
    //! キャパシティー
    size_t capacity;
    //! 個数
    size_t count;
  } elements;
  //! キーとなる文字列の最大長
  size_t maxKeySize;
  //! オブジェクトの最大サイズ
  size_t maxObjectSize;
  //! メモリプール
  void *memoryPool;

  //! @}
} Dictionary_t;

/**
 * @enum DictionaryReturn
 * @brief ステータス
 */
typedef enum DictionaryReturn {
  DICTIONARY_INVALID_ARG = -3,
  DICTIONARY_NOT_FOUND = -2,
  DICTIONARY_FULL = -1,
  DICTIONARY_ADDED = 0,
  DICTIONARY_FOUND = 1,
} DictionaryReturn;

/**
 * @brief 辞書型を初期化する
 * @param[out] self          インスタンス
 * @param[in]  capacity      格納できる最大要素数
 * @param[in]  maxKeysize    キー文字列の最大長（バイト、null終端を含む）
 * @param[in]  maxObjectSize 値オブジェクトの最大サイズ（バイト）
 */
extern void Dictionary_Init(Dictionary_t *self, size_t capacity,
                            size_t maxKeysize, size_t maxObjectSize);

/**
 * @brief 辞書にキーと値のペアを追加する
 * @param[in,out] self       インスタンス
 * @param[in]     key        キー文字列（maxKeysize以内）
 * @param[in]     object     追加する値オブジェクト
 * @param[in]     objectSize 値オブジェクトのサイズ（maxObjectSize以内）
 * @return @ref DICTIONARY_ADDED  成功
 * @return @ref DICTIONARY_FULL   容量超過
 * @return @ref DICTIONARY_INVALID_ARG 引数不正（NULL、キーまたはオブジェクトが最大サイズ超過）
 */
extern DictionaryReturn Dictionary_Add(Dictionary_t *self, const char *key,
                                       void *object, size_t objectSize);

/**
 * @brief 指定されたキーの値オブジェクトを取得する
 * @param[in] self インスタンス
 * @param[in] key  検索するキー文字列
 * @return 値オブジェクトへのポインタ。見つからない場合は NULL
 */
extern const DictionaryObject_t *Dictionary_Find(Dictionary_t *self,
                                                 const char *key);

/**
 * @brief 辞書インスタンスを破棄し、確保したリソースをすべて解放する
 * @param[in,out] self インスタンス
 */
extern void Dictionary_Destroy(Dictionary_t *self);

#ifdef __cplusplus
}
#endif
