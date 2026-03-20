# CLAUDE.md

このファイルは、このリポジトリのコードを操作する際の Claude Code (claude.ai/code) 向けのガイダンスを提供します。

## プロジェクト概要

CUtils は、一般的なシステムプログラミングタスク用のモジュールを備えた C17 の静的ユーティリティライブラリ (`libCUtils.a`) です。テストは Google Test を使用して C++20 で記述されています。

## ビルドコマンド

```bash
# 設定 (いずれかのビルドタイプを一つ選択してください)
cmake -B build -DCMAKE_BUILD_TYPE=Debug    # デバッグシンボルあり、最適化なし
cmake -B build -DCMAKE_BUILD_TYPE=Release  # -O2 (最適化あり)
cmake -B build -DCMAKE_BUILD_TYPE=Coverage # デバッグ + gcov用の --coverage
cmake -B build -DCMAKE_BUILD_TYPE=Address  # デバッグ + AddressSanitizer

# 全体をビルド
cmake --build build

# 全てのテストを実行
cd build && ctest

# 特定のテストモジュールを実行 (例: Bitset)
./build/tests/Bitset/BitsetTest

# 単一のGTestフィルターで実行
./build/tests/Bitset/BitsetTest --gtest_filter=BitsetTest.Or
```

テストの実行ファイルは `<モジュール名>Test` のパターンに従います（例: `BitsetTest`, `CsvTest`, `ThreadTest`）。

## アーキテクチャ

```text
include/   ← 公開ヘッダー (C17、すべて extern "C" でラップされています)
lib/       ← 実装 (.c ファイル、C17)
tests/     ← GTest スイート (.cpp ファイル、C++20)
```

各モジュールは `include/` と `lib/` の両方にサブディレクトリを持っています。ライブラリは `lib/` 以下のすべての `.c` ファイルを再帰的にコンパイルし、単一の静的ライブラリにします。テストは `CUtils`、`glib-2.0`、および `GTest::gtest_main` に対してリンクされます。

**外部依存関係**: `glib-2.0` (システムにインストールされている必要があります)。Google Test は CMake の `FetchContent` を通じて自動的に取得されます。

## モジュール

| モジュール | 主要な型 / エントリポイント |
|---|---|
| **Bitset** | `Bitset` 構造体; ビット演算 (And, Or, XOR, Shift, Count など) |
| **ExtendedTypes/ArrayList** | `ArrayList` — プラガブルな `MemoryAllocator` を持つ動的配列 |
| **ExtendedTypes/Dictionary** | `Dictionary` — GNU `hsearch` がバックエンドのハッシュテーブル |
| **Csv** | `CsvParser_LoadFromFile/Data` → `CsvContent` → `CsvLineCollection` → `CsvItemCollection` |
| **PublisherSubscriber** | `Broker` が `Publisher`/`Subscriber` をオーケストレーション; コールバックベースの配信 |
| **Thread/ThreadPool** | glib スレッドがバックエンドの `ThreadPool`; `ThreadPool_Push/PushTasks` でタスクをプッシュ |
| **Thread/BackGroundTask** | `setup`/`progressChanged`/`finished` コールバックを持つ単一の長時間実行タスク |
| **Process** | `Process_Execute/Start/Wait`; 標準出力/標準エラー出力をキャプチャ; タイミングとリソース使用量(rusage)を追跡 |
| **Stream/MemoryStream** | Read/Write/Seek を備えたインメモリバイトストリーム |

## 規約 (Conventions)

- すべての公開ヘッダーは、`#ifdef __cplusplus extern "C" { ... } #endif` によって C++ からのインクルードをガードしています。
- 戻り値にはモジュール固有の enum（例: `CsvReturnCode`, `SubscriberReturnCode`）を使用します。エラーに生の int 型を絶対に使用しないでください。
- ヌルポインタのガードには、分岐予測のヒントとして `__glibc_likely`/`__glibc_unlikely` を使用します。
- プライベートヘルパー（例: `lib/Utilities/utilities.h`, `lib/Process/ProcessManager.h`）は `include/` には公開されず、公開 API に追加してはなりません。
- ビルドモードは `-DCMAKE_BUILD_TYPE` を介して選択します。個別の configure スクリプトは存在しません。

## テスト構造

各モジュールのテストは `tests/<モジュール名>/` に配置されます。パターンは以下の通りです：
- `main.cpp` — テストヘッダーをインクルードし、`main()` を持ちます
- `<機能名>Test.hpp` — `SetUp`/`TearDown` を持つ `::testing::Test` フィクスチャ

`Coverage` 構成でビルドした場合、カバレッジの HTML レポートは `tests/<モジュール名>/HTML/` 配下に生成されます。
