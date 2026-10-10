# デザイナー

Tokotachi 専用の GUI デザイナー(VS Code の拡張)の設計ドキュメント。画面の定義ファイルから、
ウィンドウを継承した C++ のクラスを生成する。

方針(本リポジトリに作る理由、Bethany のデザイナーから流用するもの、キャンバスを WebAssembly の Tokotachi で描くこと)は
[ADR 0039](../adr/0039-designer.md) を参照。実装は `designer/` に置く(これから作る)。

## 構成

| ドキュメント | 内容 | 状態 |
|---|---|---|
| [dsl-spec.md](dsl-spec.md) | 画面の定義ファイルの仕様 | 草案 |
| [codegen-design.md](codegen-design.md) | コード生成(マーカー区間の更新)の設計と生成例 | 草案 |
| [catalog.md](catalog.md) | カタログ(公開のヘッダからの取り出し・既定値の測定・手書きの補足) | 草案 |

## 進める順序(ADR 0039)

1. 定義ファイルの形式・コード生成・カタログの設計(このフォルダ)。並行して WebAssembly の試作(`spike/wasm/`。描けた。ADR 0039) ← いまここ
2. カタログの取り出し
3. コード生成と CLI
4. キャンバスの描画の部分(WebAssembly)
5. VS Code の拡張とデザイナーの画面
