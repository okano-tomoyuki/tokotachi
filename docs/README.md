# ドキュメント

SDL3 を土台にした C++11 のクロスプラットフォーム GUI ライブラリ Tokotachi の設計ドキュメント。

## 構成

| ドキュメント | 内容 | 状態 |
|---|---|---|
| [adr/](adr/) | 設計判断の記録(1 判断 1 ファイル) | 随時追加 |
| [../include/tokotachi/](../include/tokotachi/) | 公開のヘッダ(API の説明はヘッダのコメント)。役割ごとのディレクトリに分ける([ADR 0035](adr/0035-header-directories.md)) | 随時追加 |
| [../spike/](../spike/) | 技術検証のコード(ライブラリには引き継がない) | マイルストーン1 |
| [designer/](designer/) | デザイナー(VS Code の拡張)の設計([ADR 0039](adr/0039-designer.md)) | 草案 |
| [bindings/](bindings/) | C の API と Python のバインディングの使い方([ADR 0065](adr/0065-c-api-and-python-bindings.md)) | 随時追加 |

`docs/` には、目的と設計判断(ADR)のように、あとから覆りにくい・背景ごと残したい情報を置く。

## ADR 一覧

| No. | タイトル | 状態 |
|---|---|---|
| [0001](adr/0001-sdl3-as-platform-layer.md) | OS との境界に SDL3 を使い、ウィジェットはすべて自前で描く | 承認 |
| [0002](adr/0002-target-platforms-and-wayland-first.md) | 対象は Windows と Linux(x86_64・aarch64)とし、Linux は Wayland を主、X11 を従にする | 承認 |
| [0003](adr/0003-cpp11.md) | C++11 で書き、公開のヘッダは C++20 でもコンパイルできるようにする | 承認 |
| [0004](adr/0004-mingw-toolchain.md) | Windows のツールチェーンは MinGW-w64(MSYS2 の mingw64)とし、実行ファイルは静的にリンクする | 承認 |
| [0005](adr/0005-build-dependencies-from-source.md) | クロスプラットフォームの依存ライブラリはソースからビルドして静的にリンクし、OS の一部はシステムのものを使う | 承認 |
| [0006](adr/0006-plutovg-cpu-rendering.md) | 2D の描画は Canvas の抽象の裏で plutovg(CPU)を使い、GPU の描画は後から別の実装として加える | 承認 |
| [0007](adr/0007-text-stack.md) | テキストは HarfBuzz + FreeType を直接使い、レイアウト・フォールバック・IME の表示は自前で持つ | 承認 |
| [0008](adr/0008-coordinate-spaces.md) | 座標は dp・window・pixel の 3 つを区別し、ウィジェットは dp で扱う | 承認 |
| [0009](adr/0009-ime-preedit-drawn-by-app.md) | IME の変換中の文字列はライブラリが入力欄の中に描き、候補窓は OS・IME に任せる | 承認 |
| [0010](adr/0010-retained-mode-and-event-loop.md) | ウィジェットは保持型のツリーとし、イベントループは待機型で、変化した領域だけを描き直す | 承認 |
| [0011](adr/0011-popups-as-child-windows.md) | メニュー・ドロップダウン・ツールチップは、親のウィンドウに相対的なポップアップのウィンドウとして作る | 承認(詳細は 0022) |
| [0012](adr/0012-ownership-and-naming.md) | ウィジェットは親が子を所有し、公開の API はクラスを PascalCase・関数を snake_case にする | 一部置換(→0013) |
| [0013](adr/0013-api-style-properties-and-handlers.md) | 公開の API はプロパティと単一のハンドラで表し、命名と書式の規則を定める | 承認 |
| [0014](adr/0014-timer.md) | タイマーは Object の派生クラスとし、enabled で動かし、利用者が寿命を管理する | 承認(一部置換→0015) |
| [0015](adr/0015-object-ownership.md) | すべての Object が所有者を持てるようにし、ウィジェットの親は所有者を兼ねる | 承認 |
| [0016](adr/0016-property-binding.md) | プロパティどうしを結びつける汎用の仕組みを入れ、Variable はその特別な場合とする | 承認 |
| [0017](adr/0017-layout.md) | 配置はウィジェットに 1 つだけ付けるレイアウトが決め、測る・置くの 2 段階で、描画の前にまとめて行う | 承認 |
| [0018](adr/0018-reparenting.md) | ウィジェットの親は付け替えられるようにし、レイアウトの add は子でないウィジェットを子にする | 承認 |
| [0019](adr/0019-line-breaking-and-text-navigation.md) | テキストのレイアウトは複数行とし、改行・書記素・単語の境界は libunibreak(UAX #14・#29)で求める | 承認 |
| [0020](adr/0020-bidi-and-text-rendering-quality.md) | 双方向テキストは SheenBidi で行の中を見た目の順に並べ、文字の描画は被覆率の補正とサブピクセルの位置決めで品質を上げる | 承認 |
| [0021](adr/0021-scrolling-and-multi-line-input.md) | スクロールはホイールのイベント・ScrollBar・ScrollArea で扱い、複数行の入力欄は LineEdit と共通の基底から作る | 承認 |
| [0022](adr/0022-popups-combo-box-menu-tooltip.md) | ポップアップは Popup とし、開いているメニューは Application がまとめて扱う。ComboBox・Menu・ツールチップをその上に作る | 承認 |
| [0023](adr/0023-theme-and-dark-mode.md) | テーマはアプリケーションが 1 つ持ち、色は役割か具体的な色で指定する。ライト・ダークは既定で OS の設定に従う | 承認 |
| [0024](adr/0024-value-widgets.md) | 値の部品(Slider・SpinBox・ProgressBar)は double で値を持ち、操作できる部品は RangeBase を基底にする | 承認 |
| [0025](adr/0025-tab-widget.md) | TabWidget はタブを Tab オブジェクトで表し、ページを子として持ち、見出しがはみ出したら横にスクロールする | 承認 |
| [0026](adr/0026-item-model-and-views.md) | 一覧・木はモデルとビューに分け、手軽に使える項目のモデルも用意する | 承認 |
| [0027](adr/0027-actions-shortcuts-and-menu-bar.md) | 操作は Action で表し、メニューの項目とショートカットが同じ Action を指す。メニューバーはウィンドウの専用の場所に置く | 承認 |
| [0028](adr/0028-dialogs.md) | ダイアログは OS の別のウィンドウとして非同期に開き、ファイルの選択は OS のダイアログを使う | 承認 |
| [0029](adr/0029-threads-and-post.md) | GUI はメインスレッドだけで触り、別のスレッドからは post で頼む。スレッドのクラスは作らず、終わりのある仕事は Task で表す | 承認 |
| [0030](adr/0030-tool-bar-and-status-bar.md) | ツールバーは Action をボタンとして並べ、ステータスバーとともにウィンドウの専用の場所に置く | 承認 |
| [0031](adr/0031-splitter.md) | Splitter は区画をいくつでも並べ、大きさの変化は区画ごとの比率で配り、区画をたためる | 承認 |
| [0032](adr/0032-canvas-api.md) | Canvas は利用者が描くための公開の API とし、決まりを明文化して、変換・線の形・パスのクリップ・手軽な関数を加える | 承認 |
| [0033](adr/0033-bitmap-paint-box-and-layers.md) | 書き換えられる画像 Bitmap・描く部品 PaintBox・グループの透過(レイヤー)を加える | 承認 |
| [0034](adr/0034-drawing-on-worker-threads.md) | フォントの処理を 1 つのロックで守り、別のスレッドで Bitmap に文字も描けるようにする | 承認 |
| [0035](adr/0035-header-directories.md) | 公開のヘッダと実装を役割ごとの 7 つのディレクトリに分ける | 承認 |
| [0036](adr/0036-library-name.md) | ライブラリの名前を Tokotachi(常立)にする | 承認 |
| [0037](adr/0037-distribution.md) | 共有ライブラリを既定として配り、CMake のパッケージ・pkg-config・ソースからの組み込みに対応する | 承認 |
| [0038](adr/0038-reference-site.md) | リファレンスは Doxygen でヘッダから作り、手引き・サンプルの一覧とともに GitHub Pages に出す | 承認 |
| [0039](adr/0039-designer.md) | デザイナーを VS Code の拡張として本リポジトリに作り、キャンバスは WebAssembly にした Tokotachi で描く | 承認 |
| [0040](adr/0040-designer-canvas-ops.md) | キャンバスとコード生成は同じ「部品を組む操作の並び」を使い、ウィンドウの画素を Window::capture で取り出す | 承認 |
| [0041](adr/0041-visible-and-container-hiding.md) | 入れ物が自分の都合で子を隠すときは visible を変えず、内部の印で隠す | 承認 |
| [0042](adr/0042-widget-theme-override.md) | ウィジェットごとにテーマの一部を上書きし、子孫に継がせる | 承認 |
| [0043](adr/0043-designer-component-files.md) | デザイナーの部品ファイル: 部品の組み合わせを別の定義ファイルにし、ほかのウィンドウから参照して置く | 承認 |
| [0044](adr/0044-designer-custom-widgets.md) | デザイナーの自作部品: 利用者が C++ で書いた部品のヘッダを読み、パレットに出して置く | 承認 |
| [0045](adr/0045-font-bold-and-italic.md) | Font に太字・斜体を加え、OS の書体のファイルを使い、無ければ合成する | 承認 |
| [0046](adr/0046-item-view-text-style-search-and-editing.md) | 一覧・木に、項目ごとの文字の色とフォント・文字での検索・その場での編集を加える | 承認 |
| [0047](adr/0047-drag-and-drop.md) | ドラッグ&ドロップはウィンドウの中と、OS からの受け取りに対応し、一覧・木の並べ替えに使う | 承認 |
| [0048](adr/0048-image-box-and-icons.md) | 画像を表示する部品 ImageBox と、ラベル・ボタン・メニューの画像を加え、拡大・縮小を滑らかにする | 承認 |
| [0049](adr/0049-group-box-mnemonics-and-widget-details.md) | 見出しの付いた枠 GroupBox・部品のニーモニックと、値の部品・分割の細かな機能を加える | 承認 |
| [0050](adr/0050-table-view.md) | 表 TableView は ItemView の表の扱いとして作り、セルの矩形を選んで表計算の流儀で編集する | 承認 |
| [0051](adr/0051-date-and-color-input.md) | 日付の値 Date と、日付の入力欄 DateEdit・暦 Calendar・色を選ぶダイアログ ColorDialog・色のボタン ColorButton を加える | 承認 |
| [0052](adr/0052-designer-band-selection-and-references.md) | デザイナーで範囲で囲んで選び、選んだものをまとめて並べ替え、参照のプロパティを編集し、はみ出した部品を示す | 承認 |
| [0053](adr/0053-designer-image-source-file.md) | デザイナーが埋め込む画像は別のソースに書き、大きい画像は警告する | 承認 |
| [0054](adr/0054-icons.md) | よく使うアイコンを Lucide の線画としてライブラリに組み込み、部品の文字の色で塗る | 承認 |
| [0055](adr/0055-extension-icon.md) | デザイナーの拡張のアイコンを、画面の部品で組んだ「常」にする | 承認 |
| [0056](adr/0056-reference-class-docs-and-widget-images.md) | リファレンスのクラスのページに説明と部品の画像を出し、クラスを分類(トピック)で引けるようにする | 承認 |
| [0057](adr/0057-compatibility-abi-and-c-layer.md) | 1.0 からの互換の約束と pimpl・ABI・公開するシンボル・将来の C の層の方針 | 承認 |
| [0058](adr/0058-class-hierarchy-and-extension-points.md) | クラスの階層を整理し、利用者が派生してよいクラス(拡張点)と標準の見た目を描く関数を決める | 承認 |
| [0059](adr/0059-api-review-before-1-0.md) | 1.0 の前に公開の API を見直し、名前・状態の表し方を揃える | 承認 |
| [0060](adr/0060-canvas-without-virtual-functions.md) | Canvas を仮想関数を持たない具体的なクラスにし、描画先を内部に隠す | 承認 |
| [0061](adr/0061-library-release.md) | ライブラリの版の出し方(タグ・変更履歴・パッケージ・利用者のプロジェクトでの確かめ) | 承認 |
| [0062](adr/0062-window-ownership-and-quit.md) | ウィンドウの所有・所有者のウィンドウ・メインのウィンドウと終わり方、オブジェクトは new で作る書き方に揃える | 承認 |
| [0063](adr/0063-text-undo.md) | 入力欄で元に戻す・やり直す | 承認 |
| [0064](adr/0064-designer-format-and-versions.md) | デザイナーの定義ファイルの形式を固定し、移し替えの仕組みとライブラリの版との対応を決める | 承認 |
| [0065](adr/0065-c-api-and-python-bindings.md) | C の API を仕様から生成して単独の共有ライブラリで配り、Python のバインディングはその上に ctypes で作る | 承認 |

ADR の書式は、`# NNNN. タイトル`、状態・日付、「背景」「検討した選択肢」「決定」「影響」とする。
覆した ADR は消さず、状態を「置換(→ 新しい番号)」にする。
