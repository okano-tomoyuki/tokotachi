# designer

Tokotachi 専用の GUI デザイナー(VS Code の拡張)。設計は [docs/designer/](../docs/designer/README.md)、方針は [ADR 0039](../docs/adr/0039-designer.md)。

| パス                 | 内容                                                                                                                                                                                                                         |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `tools/catalog/`     | 公開のヘッダからカタログを取り出す(Doxygen の XML + 手書きの補足 `overlay.json`。[catalog.md](../docs/designer/catalog.md))。既定値・望む大きさはキャンバスで測る(`measure.mts`)                                             |
| `packages/core`      | カタログ、定義ファイルのスキーマ(Zod)・意味の検証・決まった形での書き出し・木のたどり方・編集のコマンド                                                                                                                      |
| `packages/codegen`   | C++ のコード生成(マーカーの区間の置き換え。[codegen-design.md](../docs/designer/codegen-design.md))                                                                                                                          |
| `packages/cli`       | コード生成の CLI(`ttd generate <file.ttui.json \| file.ttproj.json> [--force] [--check]`)                                                                                                                                    |
| `samples/`           | 見本の定義ファイル(テストと、生成したコードのビルドでの確認に使う)                                                                                                                                                           |
| `tools/codegen/`     | 見本から生成した C++ を Tokotachi でビルド・実行して確かめる(`verify-cpp.mts`)、テストの期待する出力を作り直す(`golden.mts`)                                                                                                 |
| `canvas/`            | キャンバスの描画の部分(C++。Emscripten で WebAssembly にする。操作の並びを実行して描く。[ADR 0040](../docs/adr/0040-designer-canvas-ops.md))                                                                                 |
| `packages/canvas`    | WebAssembly のキャンバスを Node.js から使う(OS のフォントを探して渡す)                                                                                                                                                       |
| `packages/extension` | VS Code の拡張(カスタムエディタ・プロジェクト・診断・コード生成・ハンドラへの移動・JSON Schema。[editor-design.md](../docs/designer/editor-design.md))。マーケットプレースの説明は [README.md](packages/extension/README.md) |
| `packages/webview`   | デザイナーの画面(React。パレット・キャンバス・構造の木・インスペクタ)                                                                                                                                                        |
| `tools/canvas/`      | 対応表の生成(`table.mts`)、WebAssembly へのビルド(`build.mts`)、ネイティブとの画素の比較(`verify.mts`)                                                                                                                       |

## 開発

Node.js 24 と pnpm が要る。カタログの取り出しには Doxygen が要る(PATH にあるか、環境変数 `DOXYGEN` で指定する)。

```sh
cd designer
pnpm install
pnpm check                 # 型チェック・lint・書式・カタログの食い違い・テスト
pnpm build                 # Webview・拡張・CLI をビルドする(拡張は packages/extension/dist)
pnpm package               # 配る .vsix を packages/extension に作る(先に pnpm canvas:build と pnpm build)
pnpm catalog:extract       # 公開のヘッダからカタログを取り出し直す(ヘッダか overlay.json を変えたとき)
pnpm codegen:golden        # コード生成のテストの期待する出力を作り直す(生成するコードを変えたとき)
pnpm licenses              # 拡張に入れる他者のソフトウェアのライセンス(packages/extension/THIRD_PARTY_LICENSES.txt)を作り直す(依存の版を変えたとき)
pnpm codegen:verify-project # 見本からプロジェクトを作り、生成した main.cpp・CMakeLists.txt でビルドする(リポジトリの build/ のインストールを使う)
pnpm codegen:verify-cpp    # 見本から生成した C++ をビルド・実行して確かめる(C++ コンパイラ・CMake・Ninja と、ビルド済みの deps/ が要る)
pnpm canvas:table          # カタログからキャンバスの対応表を生成し直す(カタログを取り出し直したとき)
pnpm canvas:build          # キャンバスを WebAssembly にする(環境変数 EMSDK に emsdk のフォルダ。例: C:\tool\emsdk)
pnpm canvas:verify         # キャンバスで見本を描き、codegen:verify-cpp のネイティブの画像と画素を比べる
pnpm catalog:measure       # キャンバスで各クラスを作って、カタログの既定値と望む大きさを測る
node packages/cli/src/main.ts generate samples/main_window.ttui.json --comment-locale ja   # ビルドせずに CLI を動かす
```

公開のヘッダを変えたら `pnpm catalog:extract` で取り出し直し、カタログも同じコミットに含める
(`pnpm catalog:check` が食い違いを見つける)。新しい部品のクラスを足したら、`tools/catalog/overlay.json` の `classes`
(パレットに出す)か `excluded`(出さない)に書く。書き忘れると取り出しの道具がエラーにする。

画面・メッセージの日本語の訳は `packages/extension/l10n/bundle.l10n.ja.json`(CLI も使う)。文字を足したら訳も足す(`pnpm l10n:check` が漏れを見つける)。

拡張を試すには、`designer/` を VS Code で開き、実行とデバッグの「Run Extension」(F5)で開発用のホストを起動する(見本の写し `.cache/playground` が開く)。

ライブラリのビルド(リポジトリの直下の CMake)は、このフォルダに依存しない。

CI(`.github/workflows/designer.yml`)は `pnpm check` に加えて、キャンバスを WebAssembly にして Linux の Node.js でテスト・画素の比較を行い、
.vsix を作る。タグ `designer-v<版>` を push すると Marketplace に出す([docs/designer/ci.md](../docs/designer/ci.md))。
