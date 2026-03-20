\mainpage CUtils

C17 の静的ユーティリティライブラリです。一般的なシステムプログラミングタスク向けのモジュールを提供します。

## モジュール一覧

| モジュール | 説明 |
|---|---|
| **Bitset** | ビット演算 (And, Or, XOR, Shift, Count など) |
| **ArrayList** | プラガブルな `MemoryAllocator` を持つ動的配列 |
| **Dictionary** | GNU `hsearch` がバックエンドのハッシュテーブル |
| **Csv** | CSV ファイルのパースと操作 |
| **PublisherSubscriber** | コールバックベースのメッセージ配信 |
| **ThreadPool** | glib スレッドがバックエンドのスレッドプール |
| **BackGroundTask** | 単一の長時間実行タスク |
| **Process** | 外部プロセスの実行・出力キャプチャ |
| **MemoryStream** | Read/Write/Seek を備えたインメモリバイトストリーム |

## ビルド方法

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest
```

## ドキュメント生成

```bash
cmake --build build --target docs
```
