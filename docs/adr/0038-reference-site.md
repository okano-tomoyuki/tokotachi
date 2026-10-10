# 0038. リファレンスは Doxygen でヘッダから作り、手引き・サンプルの一覧とともに GitHub Pages に出す

- 状態: 承認
- 日付: 2026-10-10

## 背景

前作 Bethany は Delphi・LCL と API の体系を極力そろえ、リファレンスは本家を参照してもらう方針だった。
Tokotachi は独自の API の体系(Property・`on_xxx` のハンドラ・親が子を所有する、など)なので、その方法は取れず、自前のリファレンスが要る。

## 検討した選択肢

- 言語: 日本語で始める(選択肢A)/英語で始める(選択肢B)/両方(選択肢C)。
- 道具: Doxygen とテーマ(選択肢A)/MkDocs と Doxygen(選択肢B。Python も要る)/手書きだけ(選択肢C。ヘッダと食い違う)。
- 載せる内容: API リファレンス・使い方の手引き・設計判断(ADR)・サンプルの一覧。

## 決定

**日本語で始め、Doxygen とテーマ(doxygen-awesome-css)で作り、API リファレンス・使い方の手引き・サンプルの一覧を GitHub Pages に出す。**
ADR は載せない(リポジトリの docs/adr で読める)。

### API リファレンス

- 公開のヘッダのコメントを Doxygen が拾う形にした: 宣言の前は `///`、宣言の後ろは `///<`、ファイルの先頭は `/// @file`、
  コメントの中のコード例は `@code`〜`@endcode`。区切りの見出し(`// --- 位置と大きさ ---`)はふつうのコメントのまま残す
  (直後の宣言の説明に混ざらないように)。
- 説明の無いメンバーも載せ、private と内部(`tokotachi::detail`)は載せない。Graphviz があれば継承の図を描く。

### 手引きとサンプルの一覧

- 手引きは `docs/guide/` の Markdown(トップ・はじめに・基本の考え方・レイアウト・ウィンドウとメニューとダイアログ・一覧と木・描画・スレッド)。
- サンプルの一覧は `examples/gallery/` の部品ごとの小さなプログラム(`TOKOTACHI_BUILD_EXAMPLES` でビルド)。
  **画面の画像は、リファレンスを作るときにサンプルを画面なしで動かして撮る**(`--screenshot`。ライトのテーマ)ので、いつも最新になる。
  ソースは Doxygen の `@include` で載せる。

### 作り方

- CMake の `TOKOTACHI_BUILD_DOCS=ON` で `docs` の target を作る(`cmake --build build --target docs` → `build/docs/html`)。
  テーマの CSS は版(v2.5.0)と SHA256 を固定して取ってくる。手元でも CI でも同じ手順。
- GitHub Actions(`.github/workflows/docs.yml`)で、master に push したときに作って Pages に出す。Ubuntu の apt の Doxygen は古いので、
  公式の配布物(1.18.0)を SHA256 を固定して使う。日本語のフォント(fonts-noto-cjk)を入れて画像を撮る。依存ライブラリのビルドは
  deps/ が変わらない限りキャッシュを使い回す。

## 影響

- Pages に出す前に、GitHub のリポジトリの設定で Pages の出し方を「GitHub Actions」にする必要がある。
  リポジトリが非公開なら、Pages には有料のプランが要る。
- 英語のリファレンスは、利用者が増えてから検討する。
- 版ごとのリファレンスは、1.0 に近づいたら検討する。
