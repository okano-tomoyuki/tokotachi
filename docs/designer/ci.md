# デザイナーの CI と Marketplace への公開

デザイナー(VS Code の拡張)を確かめ、Marketplace に出すためのワークフロー([.github/workflows/designer.yml](../../.github/workflows/designer.yml))。
キャンバスの WebAssembly のモジュールはコミットしないので([ADR 0040](../adr/0040-designer-canvas-ops.md))、配る .vsix は CI で作る。

## 1. いつ動くか

| きっかけ | 動くジョブ |
|---|---|
| master への push・手での実行 | check・canvas・package |
| プルリクエスト(designer/・公開のヘッダ・src/・deps/・cmake/ を変えたもの) | check・canvas・package |
| タグ `designer-v<版>` の push | check・canvas・package・publish |

## 2. ジョブ

| ジョブ | 内容 |
|---|---|
| check | `pnpm check`(型チェック・lint・書式・カタログの食い違い・JSON Schema・キャンバスの対応表・訳・テスト)。カタログの検査に Doxygen 1.18.0 を使う(リファレンスと同じ版) |
| canvas | Emscripten 6.0.12 でキャンバスを WebAssembly にし(`pnpm canvas:build`)、**Linux の Node.js 24 で**次を確かめる。①キャンバスを使うテスト(入れる場所・つまみ。WebAssembly が無いと飛ばすもの)、②見本から生成した C++ のネイティブのビルドと実行(`codegen:verify-cpp`)、③キャンバスとネイティブの画素の比較(`canvas:verify`)、④生成したプロジェクトのビルド(`codegen:verify-project`)。フォントは両方とも fontconfig で探すので、同じフォント(Noto CJK)で描く。作ったモジュールと比べた画像を成果物に残す |
| package | canvas の WebAssembly を入れて `pnpm build`・`pnpm package` で .vsix を作り、キャンバスのモジュールと Webview が入っていることを確かめて、成果物に残す |
| publish | タグと `packages/extension/package.json` の `version` が合うことを確かめ、package の .vsix を `vsce publish` で出す |

依存ライブラリ(WebAssembly 版・ネイティブ版)のビルドは、`deps/` が変わらない限りキャッシュを使い回す。

拡張には他者のソフトウェア(WebAssembly にした Tokotachi の依存・Emscripten・musl・Lucide・まとめた JavaScript のパッケージ)が入るので、
そのライセンスを `packages/extension/THIRD_PARTY_LICENSES.txt` に入れる。`pnpm licenses` で作り直し、`pnpm check` の `licenses:check` が
古くなったことに気づく(依存の版を変えたら作り直す)。Emscripten・musl の文は `packages/extension/licenses/` に写してある
(emsdk の版を上げたら写し直す)。

最初の実行(2026-10-10、プルリクエスト #1)で、check・canvas・package がすべて通った。canvas では、生成したコードの確認 39 項目がすべて通り、
キャンバスとネイティブの画像は見本 2 つとも画素が一致した(キャッシュが無いときの canvas は約 6 分)。

## 3. 公開の手順

最初に 1 回だけ行う準備:

1. Marketplace の発行者(publisher)を作る。ID は `package.json` の `publisher`(`okano-tomoyuki`)と同じにする。
2. Azure DevOps で、Marketplace の Manage の権限を持つ個人用アクセストークンを作る。
3. リポジトリの設定で環境 `marketplace` を作り、そのシークレット `VSCE_PAT` にトークンを入れる
   (公開の前に承認を求めるなら、環境に保護の規則を付ける)。

版を出すとき:

1. `designer/packages/extension/package.json` の `version` を上げ、`CHANGELOG.md` と、対象のライブラリの版の表(README.md の「版と互換」)に
   書いてコミットする。1.0 より前は `"preview": true`、1.0 からは外す(`pnpm release:check` が確かめる。ADR 0064)。
2. `designer-v<版>`(例: `designer-v0.1.0`)のタグを付けて push する。publish のジョブが Marketplace に出す。

## 4. 未決の論点

| # | 論点 | 案 |
|---|---|---|
| D1 | Open VSX(VSCodium 等)にも出すか | 出すなら publish に `ovsx publish` を加え、シークレット `OVSX_PAT` を足す |
| D2 | Marketplace の拡張の画像(`icon`。128×128 の PNG) | 無くても出せる。用意したら `package.json` の `icon` に書き、`.vscodeignore` で含める |
