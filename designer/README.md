# designer

Tokotachi 専用の GUI デザイナー(VS Code の拡張)。設計は [docs/designer/](../docs/designer/README.md)、方針は [ADR 0039](../docs/adr/0039-designer.md)。

| パス               | 内容                                                                                                                         | 状態                         |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------- | ---------------------------- |
| `tools/catalog/`   | 公開のヘッダからカタログを取り出す(Doxygen の XML + 手書きの補足 `overlay.json`。[catalog.md](../docs/designer/catalog.md))  | 実装済み(既定値の測定はまだ) |
| `packages/core`    | カタログ、定義ファイルのスキーマ(Zod)・意味の検証・決まった形での書き出し・木のたどり方                                      | 実装済み                     |
| `packages/codegen` | C++ のコード生成(マーカーの区間の置き換え。[codegen-design.md](../docs/designer/codegen-design.md))                          | 実装済み                     |
| `packages/cli`     | コード生成の CLI(`ttd generate <file.ttui.json> [--force] [--check]`)                                                        | 実装済み                     |
| `samples/`         | 見本の定義ファイル(テストと、生成したコードのビルドでの確認に使う)                                                           |                              |
| `tools/codegen/`   | 見本から生成した C++ を Tokotachi でビルド・実行して確かめる(`verify-cpp.mts`)、テストの期待する出力を作り直す(`golden.mts`) | 実装済み                     |

## 開発

Node.js 24 と pnpm が要る。カタログの取り出しには Doxygen が要る(PATH にあるか、環境変数 `DOXYGEN` で指定する)。

```sh
cd designer
pnpm install
pnpm check                 # 型チェック・lint・書式・カタログの食い違い・テスト
pnpm build                 # CLI をビルドする(packages/cli/dist/cli.js)
pnpm catalog:extract       # 公開のヘッダからカタログを取り出し直す(ヘッダか overlay.json を変えたとき)
pnpm codegen:golden        # コード生成のテストの期待する出力を作り直す(生成するコードを変えたとき)
pnpm codegen:verify-cpp    # 見本から生成した C++ をビルド・実行して確かめる(C++ コンパイラ・CMake・Ninja と、ビルド済みの deps/ が要る)
node packages/cli/src/main.ts generate samples/main_window.ttui.json --comment-locale ja   # ビルドせずに CLI を動かす
```

公開のヘッダを変えたら `pnpm catalog:extract` で取り出し直し、カタログも同じコミットに含める
(`pnpm catalog:check` が食い違いを見つける)。新しい部品のクラスを足したら、`tools/catalog/overlay.json` の `classes`
(パレットに出す)か `excluded`(出さない)に書く。書き忘れると取り出しの道具がエラーにする。

メッセージは今は英語だけ。日本語の訳は、拡張を作るときに `packages/extension/l10n/` に置く。

ライブラリのビルド(リポジトリの直下の CMake)は、このフォルダに依存しない。
