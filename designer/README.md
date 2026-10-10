# designer

Tokotachi 専用の GUI デザイナー(VS Code の拡張)。設計は [docs/designer/](../docs/designer/README.md)、方針は [ADR 0039](../docs/adr/0039-designer.md)。

| パス             | 内容                                                                                                                        | 状態                         |
| ---------------- | --------------------------------------------------------------------------------------------------------------------------- | ---------------------------- |
| `tools/catalog/` | 公開のヘッダからカタログを取り出す(Doxygen の XML + 手書きの補足 `overlay.json`。[catalog.md](../docs/designer/catalog.md)) | 実装済み(既定値の測定はまだ) |
| `packages/core`  | カタログ(`src/catalog/catalog.json`)とその型。これから定義ファイルのスキーマ・検証・書き出し、編集のコマンドを加える        | カタログのみ                 |

## 開発

Node.js 24 と pnpm が要る。カタログの取り出しには Doxygen が要る(PATH にあるか、環境変数 `DOXYGEN` で指定する)。

```sh
cd designer
pnpm install
pnpm check             # 型チェック・lint・書式・カタログの食い違い・テスト
pnpm catalog:extract   # 公開のヘッダからカタログを取り出し直す(ヘッダか overlay.json を変えたとき)
```

公開のヘッダを変えたら `pnpm catalog:extract` で取り出し直し、カタログも同じコミットに含める
(`pnpm catalog:check` が食い違いを見つける)。新しい部品のクラスを足したら、`tools/catalog/overlay.json` の `classes`
(パレットに出す)か `excluded`(出さない)に書く。書き忘れると取り出しの道具がエラーにする。

ライブラリのビルド(リポジトリの直下の CMake)は、このフォルダに依存しない。
