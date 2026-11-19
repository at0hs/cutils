# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

このリポジトリのコードを操作する際の Claude Code (claude.ai/code) 向けのガイダンスを提供します。

## プロジェクト概要

CUtils は、一般的なシステムプログラミングタスク用のモジュールを備えた C17 の静的ユーティリティライブラリ (`libCUtils.a`) です。テストは Google Test を使用して C++20 で記述されています。

## ビルドコマンド

```bash
# 設定 (いずれかのビルドタイプを一つ選択してください)
cmake -B build -DCMAKE_BUILD_TYPE=Debug       # デバッグシンボルあり、最適化なし
cmake -B build_cov -DCMAKE_BUILD_TYPE=Coverage  # デバッグ + gcov用の --coverage
cmake -B build_asan -DCMAKE_BUILD_TYPE=Address  # デバッグ + AddressSanitizer
cmake -B build -DCMAKE_BUILD_TYPE=Release     # -O2 (最適化あり)

# 全体をビルド
cmake --build build

# 全てのテストを実行 (gtest_discover_tests により各テストケースが個別に登録される)
cd build && ctest
ctest --test-dir build -j8 --output-on-failure

# 特定のテストモジュールを実行 (例: Bitset)
./build/tests/Bitset/BitsetTest

# 単一のGTestフィルターで実行
./build/tests/Bitset/BitsetTest --gtest_filter=BitsetTest.Or

# Doxygen ドキュメントを生成 (Doxygen がインストールされている場合)
cmake --build build --target docs
```

テストの実行ファイルは `<モジュール名>Test` のパターンに従います（例: `BitsetTest`, `CsvTest`, `ThreadTest`）。

## アーキテクチャ

```text
include/   ← 公開ヘッダー (C17、すべて extern "C" でラップされています)
lib/       ← 実装 (.c ファイル、C17)
tests/     ← GTest スイート (.cpp ファイル、C++20)
```

各モジュールは `include/` と `lib/` の両方にサブディレクトリを持っています。ライブラリは `lib/` 以下のすべての `.c` ファイルを再帰的にコンパイルし、単一の静的ライブラリにします。テストは `CUtils`、`Threads::Threads`、および `GTest::gtest_main` に対してリンクされます。サニタイザ/カバレッジのリンクオプションは `CUtils` から `PUBLIC` で伝播します (`asan`/`gcov` を直接リンクしないこと)。

**外部依存関係**: POSIX スレッド (`find_package(Threads)`) のみ。Google Test は CMake の `FetchContent` を通じて自動的に取得されます。

## モジュール

| モジュール | 主要な型 / エントリポイント |
|---|---|
| **Bitset** | `Bitset` 構造体; ビット演算 (And, Or, XOR, Shift, Count など) |
| **ExtendedTypes/ArrayList** | `ArrayList` — プラガブルな `MemoryAllocator` を持つ動的配列 |
| **ExtendedTypes/LinkedList** | `LinkedList_t` — プラガブルな `MemoryAllocator` を持つ片方向連結リスト |
| **ExtendedTypes/Dictionary** | `Dictionary_t` — GNU `hsearch` がバックエンドのハッシュテーブル |
| **ExtendedTypes/MemoryAllocator** | `MemoryAllocator_t` — `ArrayList`/`LinkedList` に渡すカスタムアロケータインタフェース; `NULL` 渡しで `malloc`/`free` |
| **Csv** | `CsvParser_LoadFromFile/Data` → `CsvContent` → `CsvLineCollection` → `CsvLine` → `CsvItemCollection` → `CsvItem` |
| **PublisherSubscriber** | `Broker` が `Publisher`/`Subscriber` をオーケストレーション; コールバックベースの配信; `SUBSCRIBER_NACK` で `MessageStorage` にリトライキュー |
| **Thread/ThreadPool** | pthread がバックエンドの `ThreadPool`; `ThreadPool_Push/PushTasks` でタスクをプッシュ; 戻り値は `ThreadPoolReturnCode` |
| **Thread/BackGroundTask** | `setup`/`progressChanged`/`finished` コールバックを持つ単一の長時間実行タスク; `Cancel` (協調的) / `Abort` (pthread_cancel) |
| **Process** | `Process_Execute/Start/Wait`; 子プロセスごとの待機スレッドで終了検知 (SIGCHLD 不使用); 標準出力/標準エラー出力を `mkdtemp` の一時ディレクトリにキャプチャ; タイミングとリソース使用量(rusage)を追跡 |
| **Stream/MemoryStream** | `open_memstream` がバックエンドの Read/Write/Seek を備えたインメモリバイトストリーム |

## 規約 (Conventions)

- すべての公開ヘッダーは、`#ifdef __cplusplus extern "C" { ... } #endif` によって C++ からのインクルードをガードしています。
- 戻り値にはモジュール固有の enum（例: `CsvReturnCode`, `SubscriberReturnCode`, `DictionaryReturn`, `MemoryStreamReturnCode`, `ThreadPoolReturnCode`）を使用します。エラーに生の int 型を絶対に使用しないでください。
- ヌルポインタのガードには、分岐予測のヒントとして `__glibc_likely`/`__glibc_unlikely` を使用します。
- プライベートヘルパー（例: `lib/Utilities/utilities.h`）は `include/` には公開されず、公開 API に追加してはなりません。
- ビルドモードは `-DCMAKE_BUILD_TYPE` を介して選択します。個別の configure スクリプトは存在しません。

## テスト構造

各モジュールのテストは `tests/<モジュール名>/` に配置されます。パターンは以下の通りです：
- `<機能名>Test.cpp` — テストスイートの実装
- `<機能名>Test.hpp` — `SetUp`/`TearDown` を持つ `::testing::Test` フィクスチャ

`main()` は `GTest::gtest_main` が提供します（`main.cpp` は存在しません）。

モジュールによっては複数の `.cpp` をひとつの実行ファイルにまとめます（例: `ExtendedTypesTest` = ArrayList + Dictionary + LinkedList、`ThreadTest` = ThreadPool + BackGroundTask、`PublisherSubscriberTest` = Subscription + MessageStorage + PubSub）。

テスト実行ファイル名の対応：

| テストディレクトリ | 実行ファイル |
|---|---|
| `tests/Bitset/` | `BitsetTest` |
| `tests/ExtendedTypes/` | `ExtendedTypesTest` |
| `tests/Csv/` | `CsvTest` |
| `tests/Thread/` | `ThreadTest` |
| `tests/Process/` | `ProcessTest` |
| `tests/Stream/` | `StreamTest` |
| `tests/PublisherSubscriber/` | `PublisherSubscriberTest` |

`Coverage` 構成でビルドした場合（`build_cov` ディレクトリ推奨）、カバレッジの HTML レポートは `tests/<モジュール名>/HTML/` 配下に生成されます。
