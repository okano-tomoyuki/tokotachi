# ドキュメント

SDL3 を土台にした C++11 のクロスプラットフォーム GUI ライブラリの設計ドキュメント。

## 構成

| ドキュメント | 内容 | 状態 |
|---|---|---|
| [adr/](adr/) | 設計判断の記録(1 判断 1 ファイル) | 随時追加 |
| [../include/gui/](../include/gui/) | 公開のヘッダ(API の説明はヘッダのコメント) | 土台の層 |
| [../spike/](../spike/) | 技術検証のコード(ライブラリには引き継がない) | マイルストーン1 |

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

ADR の書式は、`# NNNN. タイトル`、状態・日付、「背景」「検討した選択肢」「決定」「影響」とする。
覆した ADR は消さず、状態を「置換(→ 新しい番号)」にする。
