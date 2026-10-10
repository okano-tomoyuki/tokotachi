# 0040. キャンバスとコード生成は同じ「部品を組む操作の並び」を使い、ウィンドウの画素を Window::capture で取り出す

- 状態: 承認
- 日付: 2026-10-10

## 背景

[ADR 0039](0039-designer.md) で、デザイナーのキャンバスは WebAssembly にした Tokotachi で描くと決めた。実装にあたって、次のことを決める必要がある。

- キャンバスは、生成したコード(`tokotachi_setup_ui`)と同じ手順で部品を組まなければならない(同じでなければ、デザイナーで見た画面と、
  ビルドしたアプリの画面がずれる)。
- 描いた画素を、ファイルを通さずに拡張へ渡す必要がある。今の公開の API には、ウィンドウを PNG に保存する `save_png` しか無い。
- ネイティブの Tokotachi は、いくつかのことを OS から得る(フォント、ダイアログのボタンの並び)。WebAssembly では、その OS が分からない。

## 検討した選択肢

### 組み立ての手順

- 選択肢A: 定義ファイルを C++ で読んで組む処理を、キャンバスの中に書く。コード生成(TypeScript)と同じ規則を 2 つの言語で持つことになり、ずれやすい。
- 選択肢B: コード生成を「部品を組む操作の並び」(作る・プロパティを設定する・呼び出す・置く)を作る部分と、それを C++ の文にする部分に分ける。
  キャンバスは同じ並びを受け取って実行するだけにする。

### 画素の取り出し

- 選択肢A: キャンバスの中から、ライブラリの内部(WindowImpl の描画先)を読む。
- 選択肢B: 公開の API に `Bitmap Window::capture()`(全体を描いて Bitmap にする)を加える。利用者もスクリーンショット・テストに使える。

## 決定

**組み立ては選択肢B、画素の取り出しも選択肢B。**

- **操作の並び**([designer/packages/codegen/src/model.ts](../../designer/packages/codegen/src/model.ts) の `buildSetup`)。
  - 操作は `new`(作る)・`set`(プロパティ)・`connect`(ハンドラ)・`call`(add・add_tab 等の呼び出しと、その結果の変数)・
    `place`(PlaceLayout)・`blank`(C++ の空行)。2026-10-10 に `variable`(Variable を作る)・`bind`(プロパティを Variable に結びつける)を加えた
    (キャンバスは Variable を作らず、値を覚えて、結びつけを値の設定として実行する。描くときには値が変わらないので同じ結果になる)。
  - C++ の書き出しは、この並びを文にする。分けた後も、生成するコードは 1 文字も変わらないことを、期待する出力のテストで確かめた。
  - キャンバス([designer/canvas/](../../designer/canvas/))は、並びを JSON で受け取って実行する。名前から部品を作る・プロパティを名前で
    読み書きする対応表は、カタログから生成する(`designer/canvas/src/generated/`。`pnpm canvas:table`。食い違いは `pnpm check` で見つける)。
  - JSON の読み書きは、キャンバスの中の小さな実装で行い、依存ライブラリは増やさない。
- **`Bitmap Window::capture()`** を公開の API に加える。表示していなくても全体を描き、ピクセルの大きさの Bitmap にする。
- **OS から得るものは、拡張が渡す**(`ttc_configure`)。
  - フォント: ネイティブの Tokotachi(src/graphics/font_cache.cpp)と同じ順で探したファイル(Windows は Fonts フォルダの既知のファイル、
    Linux は fontconfig の `fc-match`)。
  - ダイアログのボタンの並び: WebAssembly のときだけ、環境変数 `TOKOTACHI_HOST_OS`(`windows` なら決定のボタンが先)と
    `XDG_CURRENT_DESKTOP` を見る。
- WebAssembly のモジュール(約 3 MB)はコミットせず、`pnpm canvas:build` で作る(Emscripten が要る。拡張を配るときは CI で作る)。
- カタログの既定値と望む大きさは、同じモジュールで各クラスを作って測る(`pnpm catalog:measure`。327 個)。

## 結果の確認(2026-10-10、Windows)

- 見本 2 つ(メインのウィンドウ・設定のダイアログ)を、キャンバスで描いた画像と、生成したコードをネイティブ(MinGW)でビルドして描いた画像は、
  **1 画素も違わなかった**(`pnpm canvas:verify`)。
- 描く時間は、定義ファイルから組み直して描いて約 10 ms(部品 20 個のウィンドウ)、ダイアログは約 3 ms。最初の 1 回はフォントの読み込みを含めて約 100 ms。

## 影響

- コード生成を変えるときは、model.ts(操作の並び)を変え、C++ の書き出しとキャンバスの実行の両方に反映する。
  新しい呼び出し(`call` の method)を加えたら、キャンバスの `Runner::call` にも加える。
- 公開のヘッダを変えたら、カタログの取り出し(`pnpm catalog:extract`)と、対応表の生成(`pnpm canvas:table`)を行う。
- Linux の Node.js でのキャンバスの確認は、まだ行っていない(WSL に Node.js が無い。拡張の CI で行う)。
