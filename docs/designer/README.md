# デザイナー

Tokotachi 専用の GUI デザイナー(VS Code の拡張)の設計ドキュメント。画面の定義ファイルから、
ウィンドウを継承した C++ のクラスを生成する。

方針(本リポジトリに作る理由、Bethany のデザイナーから流用するもの、キャンバスを WebAssembly の Tokotachi で描くこと)は
[ADR 0039](../adr/0039-designer.md) を参照。実装は [designer/](../../designer/README.md) に置く。

## 構成

| ドキュメント | 内容 | 状態 |
|---|---|---|
| [dsl-spec.md](dsl-spec.md) | 画面の定義ファイルの仕様 | 実装済み(スキーマ・検証・書き出し) |
| [codegen-design.md](codegen-design.md) | コード生成(マーカー区間の更新)の設計と生成例 | 実装済み(Tokotachi でのビルド・実行で確認) |
| [editor-design.md](editor-design.md) | VS Code の拡張とデザイナーの画面(編集のしくみ・キャンバス・インスペクタ・コード生成) | 実装済み |
| [project-spec.md](project-spec.md) | プロジェクトファイル(`*.ttproj.json`)と、起動部分・CMakeLists.txt の生成 | 実装済み |
| [catalog.md](catalog.md) | カタログ(公開のヘッダからの取り出し・既定値の測定・手書きの補足) | 実装済み |
| [ci.md](ci.md) | CI(WebAssembly のビルド・Linux でのキャンバスの確認・.vsix)と Marketplace への公開 | ワークフローを用意した(まだ CI で動かしていない) |

## 進める順序(ADR 0039)

1. 定義ファイルの形式・コード生成・カタログの設計(このフォルダ)。並行して WebAssembly の試作(`spike/wasm/`。描けた。ADR 0039) ← 済み
2. カタログの取り出し(`designer/tools/catalog/`) ← 済み
3. コード生成と CLI(`designer/packages/codegen`・`cli`) ← 済み
4. キャンバスの描画の部分(WebAssembly。`designer/canvas`・`packages/canvas`。[ADR 0040](../adr/0040-designer-canvas-ops.md)) ← 済み
5. VS Code の拡張とデザイナーの画面(3 段階。[editor-design.md](editor-design.md)) ← 済み
