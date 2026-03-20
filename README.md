# CUtils

C17 で書かれた汎用システムプログラミング向け静的ユーティリティライブラリ (`libCUtils.a`) です。

## 必要環境

- CMake 3.5 以上
- C17 対応コンパイラ (GCC / Clang)
- `glib-2.0` (システムにインストールされている必要があります)
- Google Test (CMake の FetchContent で自動取得)

```bash
# Ubuntu / Debian
sudo apt install libglib2.0-dev
```

## ビルド

```bash
# 設定 (ビルドタイプを一つ選択)
cmake -B build -DCMAKE_BUILD_TYPE=Debug    # デバッグシンボルあり、最適化なし
cmake -B build -DCMAKE_BUILD_TYPE=Release  # -O2 最適化あり
cmake -B build -DCMAKE_BUILD_TYPE=Coverage # gcov カバレッジ計測用
cmake -B build -DCMAKE_BUILD_TYPE=Address  # AddressSanitizer 有効

# ビルド
cmake --build build
```

## テスト

```bash
# 全テストを実行
cd build && ctest

# モジュール単位で実行 (例: Bitset)
./build/tests/Bitset/BitsetTest

# フィルター指定で実行
./build/tests/Bitset/BitsetTest --gtest_filter=BitsetTest.Or
```

Coverage ビルドの場合、HTML カバレッジレポートが `tests/<モジュール名>/HTML/` に生成されます。

## ディレクトリ構成

```
include/   ← 公開ヘッダー (C17、extern "C" でラップ済み)
lib/       ← 実装 (.c ファイル)
tests/     ← Google Test スイート (.cpp / C++20)
```

## モジュール一覧

### Bitset

`include/Bitset/Bitset.h`

任意ビット幅のビット集合演算。

```c
Bitset_t bs;
Bitset_Init(&bs, 128);          // 128 ビットの Bitset を初期化
Bitset_Set(&bs, 5);             // ビット 5 をセット
Bitset_Or(&bs, &other);         // OR 演算
size_t n = Bitset_Count(&bs);   // セットビット数
Bitset_Destroy(&bs);
```

主な関数: `Init` / `Destroy` / `Set` / `Reset` / `Flip` / `And` / `Or` / `ExclusiveOr` / `LeftShift` / `RightShift` / `Count` / `Test` / `Any` / `All` / `None` / `ToString`

---

### ArrayList

`include/ExtendedTypes/ArrayList.h`

プラガブルなメモリアロケータを持つ動的配列。

```c
ArrayList list;
ArrayList_Init(&list, sizeof(int), 16, NULL);  // NULL = デフォルト malloc/free
int v = 42;
ArrayList_Add(&list, &v);
int *p = ArrayList_At(&list, 0);
ArrayList_Destroy(&list);
```

主な関数: `Init` / `Destroy` / `Add` / `AddReference` / `AddRange` / `Append` / `Insert` / `At` / `IndexOf` / `Contains` / `Remove` / `RemoveAt` / `Clear` / `ForEach`

---

### LinkedList

`include/ExtendedTypes/LinkedList.h`

プラガブルなメモリアロケータを持つ片方向連結リスト。

```c
LinkedList_t list;
LinkedList_Init(&list, sizeof(int), NULL);
int v = 10;
LinkedList_Add(&list, &v);
int *p = LinkedList_At(&list, 0);
LinkedList_Destroy(&list);
```

主な関数: `Init` / `Destroy` / `Add` / `At` / `Contains` / `Remove` / `RemoveAt` / `Clear` / `Count` / `IsEmpty` / `Foreach`

---

### Dictionary

`include/ExtendedTypes/Dictionary.h`

GNU `hsearch` をバックエンドとしたハッシュテーブル。

```c
Dictionary_t dict;
Dictionary_Init(&dict, 64, 32, sizeof(int));  // 容量, 最大キー長, 最大値サイズ

int val = 99;
Dictionary_Add(&dict, "key", &val, sizeof(int));

DictionaryObject_t found;
Dictionary_Find(&dict, "key", &found);

Dictionary_Destroy(&dict);
```

戻り値: `DictionaryReturn` enum (`DICTIONARY_ADDED` / `DICTIONARY_FOUND` / `DICTIONARY_NOT_FOUND` / `DICTIONARY_FULL` / `DICTIONARY_INVALID_ARG`)

---

### MemoryAllocator

`include/ExtendedTypes/MemoryAllocator.h`

`ArrayList` / `LinkedList` に渡すカスタムアロケータインタフェース。`NULL` 渡しで `malloc`/`free` を使用するデフォルト動作になります。

```c
MemoryAllocator_t allocator;
MemoryAllocator_Init(&allocator, my_alloc, my_free);  // カスタム
// または
MemoryAllocator_t *def = MemoryAllocator_GetDefault();
```

---

### CsvParser

`include/Csv/`

CSV ファイル / 文字列のパース。

```c
CsvProperties_t props = { .hasHeader = true, .eolCode = CSV_EOL_CRLF };
CsvParser_t parser;
CsvParser_Init(&parser, &props);

CsvContent_t content;
CsvParser_LoadFromFile(&parser, "data.csv", &content);

// 行・セルのアクセス
CsvLineCollection_t *lines = CsvContent_GetLines(&content);
// ...

CsvContent_Destroy(&content);
CsvParser_Destroy(&parser);
```

型階層: `CsvParser` → `CsvContent` → `CsvLineCollection` → `CsvLine` → `CsvItemCollection` → `CsvItem`

戻り値: `CsvReturnCode` enum (`CSV_SUCCESS` / `CSV_INVALID_PARAMETER` / `CSV_INVALID_CHAR_CODE`)

---

### Publisher / Subscriber

`include/PublisherSubscriber/`

属性フィルタリングとリトライキューを持つコールバックベースのメッセージ配信。

```c
Publisher_t pub;
Publisher_Init(&pub, 16);  // 最大 16 サブスクライバー

Subscriber_t sub;
void on_message(PublishContent_t *content, void *userData) { /* ... */ }
Subscriber_Init(&sub, on_message, userData);

SubscriptionAccountId id = Publisher_Subscribe(&pub, &sub, MY_ATTRIBUTE);
Publisher_Publish(&pub, message, MY_ATTRIBUTE);
Publisher_Unsubscribe(&pub, id);
Publisher_Destroy(&pub);
```

サブスクライバーが `SUBSCRIBER_NACK` を返すと、`MessageStorage` に保存されて次回配信時にリトライされます。

---

### ThreadPool

`include/Thread/ThreadPool.h`

glib スレッドをバックエンドとしたワーカースレッドプール。

```c
ThreadPool_t pool;
ThreadPool_Init(&pool, 4);  // 0 = CPU コア数

ThreadPoolTask_t task = { .func = my_func, .arg = my_arg };
ThreadPool_Push(&pool, &task);

ThreadPool_Destroy(&pool, true);  // true = タスクをすべて完了してから終了
```

主な関数: `Init` / `Destroy` / `Push` / `PushTasks` / `GetNumTasks`

---

### BackGroundTask

`include/Thread/BackGroundTask.h`

ライフサイクルコールバック付きの単一長時間実行タスク。

```c
BackGroundTask_t task;
BackGroundTaskEventHandlers_t handlers = {
    .setup           = on_setup,
    .progressChanged = on_progress,
    .finished        = on_finished,
};
BackGroundTaskWork_t work = { .func = my_work, .sender = &task };
BackGroundTask_Init(&task, "MyTask", &handlers, &work);
BackGroundTask_Run(&task);

// 作業関数内でキャンセル確認
if (task->cancellationPending) { /* 中断処理 */ }

BackGroundTask_Cancel(&task);  // キャンセル要求 (協調的)
BackGroundTask_Abort(&task);   // 強制終了 (pthread_cancel)
BackGroundTask_Destroy(&task);
```

終了ステータス: `NONE` / `EXITED` / `CANCELED` / `ABORTED` / `FAILURE`

---

### Process

`include/Process/Process.h`

サブプロセスの起動・管理・stdout/stderr キャプチャ・rusage 計測。

```c
Process proc;
ProcessStartInfo info = {
    .commandLine          = "ls -la",
    .environment          = NULL,
    .disableOutputCapture = false,
};
Process_Execute(&proc, &info);  // Init + Start の短縮形
Process_Wait(&proc);

printf("exit: %d\n",  Process_ExitCode(&proc));
printf("cpu:  %.3fs\n", Process_TotalProcessorTime(&proc));

ProcessOutput *out = Process_StandardOutput(&proc);
// out->path にキャプチャされた標準出力の一時ファイルパス

Process_Destroy(&proc);
```

主な関数: `Init` / `Destroy` / `Execute` / `Start` / `Wait` / `WaitFor` / `HasExited` / `ExitCode` / `Id` / `StartTime` / `ExitTime` / `TotalProcessorTime` / `UserProcessorTime` / `StandardOutput` / `StandardError` / `PrintSummary`

---

### MemoryStream

`include/Stream/MemoryStream.h`

`open_memstream` をバックエンドとしたインメモリ Read / Write / Seek ストリーム。

```c
MemoryStream ms;
MemoryStream_Init(&ms);

uint8_t buf[] = {1, 2, 3};
MemoryStream_Write(&ms, buf, sizeof(buf));
MemoryStream_Seek(&ms, 0, SEEK_SET);

uint8_t out[3];
MemoryStream_Read(&ms, out, sizeof(out));

size_t sz = MemoryStream_GetSize(&ms);
const uint8_t *data = MemoryStream_RefBuffer(&ms);  // 読み取り専用

MemoryStream_Destroy(&ms);
```

戻り値: `MemoryStreamReturnCode` enum (`SUCCESS` / `INVALID_ARG` / `OPEN_FAILED` / `SEEK_ERROR`)

---

## 規約

- 公開ヘッダーはすべて `extern "C"` でラップ済み (C++ から直接インクルード可能)
- 戻り値はモジュール固有の enum を使用 (生の `int` は使用しない)
- ヌルポインタガードには `__glibc_likely` / `__glibc_unlikely` を使用
- プライベートヘルパー (`lib/Utilities/`, `lib/Process/ProcessManager.h`) は `include/` に公開しない
