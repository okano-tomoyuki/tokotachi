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
| [ci.md](ci.md) | CI(WebAssembly のビルド・Linux でのキャンバスの確認・.vsix)と Marketplace への公開 | 実装済み(2026-10-10 に CI で通った) |

## 版と互換([ADR 0064](../adr/0064-designer-format-and-versions.md))

### ライブラリの版との対応

デザイナー(拡張)の版はライブラリと別に上げるが、大きい版はそろえる(デザイナー 1.x は Tokotachi 1.x 用)。各版が対象にするライブラリの版は、
カタログを取り出した版(major.minor)で、生成する `CMakeLists.txt` の `find_package` が求め、プロジェクトの設定画面にも出る。
`pnpm check` の中の `release:check` が、大きい版がそろっていることと、プレビューの印(1.0 より前だけ `"preview": true`)を確かめる。

| デザイナー | 対象の Tokotachi | 定義ファイルの形式(`format_version`) |
|---|---|---|
| 0.1.0 | 0.1 | 画面 1・プロジェクト 1 |
| 0.2.0 | 0.1 | 画面 1・プロジェクト 1 |
| 次の版 | 0.3 | 画面 1・プロジェクト 1 |

デザイナーの版を出すときは、この表に行を足す。

### 定義ファイルの形式

- `*.ttui.json`・`*.ttproj.json` の形式は、1.0 で版 1 を固定する。形式を変えるとき(項目を足すだけでも)は `format_version` を 1 つ上げ、
  前の版から移す関数を `TTUI_MIGRATIONS`(`packages/core/src/dsl/parse.ts`)・`TTPROJ_MIGRATIONS`(`packages/core/src/project/parse.ts`)に足す。
  仕様のドキュメント(dsl-spec.md・project-spec.md)の題の版も上げる。
- 読むときに、ファイルの版から今の版まで順に移し、移したことを警告(`migrated-format`)で知らせる。保存すると今の版で書き出す。
- 今の版より新しいファイルは開かず、デザイナーの更新を促す(`unsupported-version`)。
- Tokotachi の API の改名(`renamed.ts`)は形式の版とは別に、古い名前をいつまでも読み替える。

## 進める順序(ADR 0039)

1. 定義ファイルの形式・コード生成・カタログの設計(このフォルダ)。並行して WebAssembly の試作(`spike/wasm/`。描けた。ADR 0039) ← 済み
2. カタログの取り出し(`designer/tools/catalog/`) ← 済み
3. コード生成と CLI(`designer/packages/codegen`・`cli`) ← 済み
4. キャンバスの描画の部分(WebAssembly。`designer/canvas`・`packages/canvas`。[ADR 0040](../adr/0040-designer-canvas-ops.md)) ← 済み
5. VS Code の拡張とデザイナーの画面(3 段階。[editor-design.md](editor-design.md)) ← 済み
