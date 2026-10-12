# 変更履歴

ライブラリ(Tokotachi 本体)の変更履歴。デザイナーの変更履歴は
[designer/packages/extension/CHANGELOG.md](designer/packages/extension/CHANGELOG.md) にある。

版の付け方と互換の約束は [ADR 0057](docs/adr/0057-compatibility-abi-and-c-layer.md)、出し方は
[ADR 0061](docs/adr/0061-library-release.md) を参照。0.x の間は、小さい版が上がると利用者のコードの修正が要ることがある。

## 次の版(未公開)

加えたもの([ADR 0062](docs/adr/0062-window-ownership-and-quit.md)):

- 所有者を渡さずに作ったトップレベルのウィンドウは `Application` が所有し、`Application` の破棄で破棄する。
- 所有者のウィンドウを渡して作るウィンドウ `Window(owner, title, width, height)`(所有者の上に出る。所有者と一緒に隠れ、破棄される)。
- `Application::main_window`(閉じたら終える)・`Application::auto_quit_enabled`(false なら `quit()` まで終えない)。
- `Window::destroy_on_close`(閉じたら破棄する)。
- 入力欄(LineEdit・TextEdit)で元に戻す・やり直す(Ctrl+Z・Ctrl+Y。`undo`・`redo`・`can_undo`・`can_redo`。[ADR 0063](docs/adr/0063-text-undo.md))。
- サンプル・手引きを、Object から派生したものは `new` で作り、スタックに置くのは `Application` だけにする書き方に揃えた。
  デザイナーが生成する `main.cpp` も、メインのウィンドウを `new` で作り `app.main_window` に代入する。

## 0.2.0

最初に公開する版(プレビュー)。1.0 に向けて、公開の API と ABI の形を整えた。

### できること

- 部品: ボタン(Button・CheckBox・RadioButton)・ラベル・入力欄(LineEdit・TextEdit。IME・双方向のテキスト)・
  値の部品(Slider・SpinBox・ProgressBar)・ComboBox・日付(DateEdit・Calendar)・ColorButton・タブ・分割・スクロール・
  PaintBox・ImageBox・GroupBox。
- 一覧・木・表(ListView・TreeView・TableView)とモデル。選択・その場での編集・並べ替え・ドラッグ&ドロップ。
- ウィンドウ・ポップアップ・ダイアログ(メッセージ・ファイル・色)・メニューバー・ツールバー・ステータスバー・Action・ショートカット。
- レイアウト(BoxLayout・GridLayout・PlaceLayout)、プロパティと変数の結びつけ、テーマとダークモード・部品ごとのテーマの上書き。
- 描画: Canvas(パス・グラデーション・変換・クリップ・グループの透過)、Bitmap(別のスレッドでも描ける)、画像・アイコン(Lucide)。
- 別のスレッドからメインスレッドへ処理を頼む post と、終わりのある仕事 Task。
- Windows(MinGW-w64)・Linux(Wayland・X11、x86_64・aarch64)。共有ライブラリと静的ライブラリ、CMake のパッケージ・pkg-config。

### 0.1.0(リポジトリの中の版)からの変更

利用者のコードの修正が要るもの:

- 改名([ADR 0059](docs/adr/0059-api-review-before-1-0.md)): `Label::color` → `text_color`、`TextInputBase::deselect()` →
  `clear_selection()`、`Timer::on_timer` → `on_timeout`、`ThemeOverride::empty()`・`Path::empty()` → `is_empty()`、
  `FileDialog::allow_many` → `multi_enabled`。
- 状態を読む関数を、読むだけのプロパティにした: `ComboBox::popup_open`・`DateEdit::calendar_open`・`ItemView::editing`・
  `Menu::opened`(変わったときに知らせるので、変数と結びつけられる)。
- 公開のクラスは実装を内部に隠した(pimpl。[ADR 0057](docs/adr/0057-compatibility-abi-and-c-layer.md))。private のメンバや
  `friend` を前提にしたコードは使えない。
- 共有ライブラリは、公開の API(`TOKOTACHI_API`)だけを外に見せる。内部の関数を直接呼んでいたコードはリンクできない。
- `Canvas` は利用者が派生させるクラスではなくなった(仮想関数を持たない。[ADR 0060](docs/adr/0060-canvas-without-virtual-functions.md))。
- 内部の仕組みのための仮想関数を公開のクラスから外し、部品の階層を整理した([ADR 0058](docs/adr/0058-class-hierarchy-and-extension-points.md))。
  自作の部品は `ButtonBase`・`RangeBase` などの拡張のための基底か、具体的な部品から派生させる。

加えたもの:

- 標準の見た目を描く関数 `tokotachi::draw`(ボタンの面・チェックやラジオの印・フォーカスの枠・入力欄の枠)と `ControlState`、
  `ButtonBase::state()`。自作の部品で標準の見た目を使える。
- 手引きに「自作の部品」を加えた。

### 配布

- SONAME は `libtokotachi.so.0.2`(0.x の間は小さい版ごとに変わる)。
- GitHub の Release に、Linux(x86_64・aarch64。Ubuntu 24.04 でビルド)と Windows(MSYS2 の MINGW64)の、共有ライブラリ・
  静的ライブラリのパッケージを添える。ビルドした環境は各パッケージの `BUILD_INFO.txt` にある。
