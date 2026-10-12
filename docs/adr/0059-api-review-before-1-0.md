# 0059. 1.0 の前に公開の API を見直し、名前・状態の表し方を揃える

- 状態: 承認
- 日付: 2026-10-13

## 背景

1.0 からは、公開の API の名前を変えると利用者のコードを壊す([ADR 0057](0057-compatibility-abi-and-c-layer.md))。その前に、公開の
ヘッダの型(97 個)と公開のメンバ(776 個)を横に並べ、名前の付け方・型・引数の揃い方を見直した。

揃っていると確かめたもの:

- コンストラクタの形(部品は `(Widget* parent = nullptr, …)`、レイアウトは `(Widget* target)`、Object は `(Object* owner = nullptr, …)`、
  ダイアログは `(Window* parent, …)`)。
- ハンドラの名前(`on_` と現在形。Popup の `on_closed` は、継いでいる Window の `on_close`(閉じる前に止められる)と区別するため)。
- 追加・削除の名前(`add_X`・`insert_X`・`remove_X`)、数の型(値は `double`、位置と大きさは `float` の dp、時間は `int` のミリ秒)、列挙。
- 倍率の読み取り(`Canvas::scale()`・`Bitmap::scale()`・`TextLayout::scale()` はどれも dp からピクセルへの倍率)。

## 決定

### 名前を揃える(改名)

| 前 | 後 | 理由 |
|---|---|---|
| `Label::color` | `Label::text_color` | `Item::text_color` と揃え、`ColorButton::color`(選んだ色という値)と取り違えにくくする |
| `TextInputBase::deselect()` | `clear_selection()` | `ItemView::clear_selection()` と揃える |
| `Timer::on_timer` | `on_timeout` | 上書きする仮想関数 `timeout()` と揃える |
| `ThemeOverride::empty()`・`Path::empty()` | `is_empty()` | ほかの `is_empty`・`is_null`・`is_valid` と揃える |
| `FileDialog::allow_many` | `multi_enabled` | 真偽のプロパティの `*_enabled` と揃える |

### 状態は読むだけのプロパティにする

状態はプロパティで読む([ADR 0013](0013-api-style-properties-and-handlers.md))のに、関数で返していたものを、読むだけのプロパティにした。
変わったときに知らせるので、変数と結びつけられる。

| 前 | 後 | 知らせるとき |
|---|---|---|
| `ComboBox::is_popup_open()` | `popup_open` | 一覧を開いたとき、閉じたとき(外側を押して閉じたときも) |
| `DateEdit::is_calendar_open()` | `calendar_open` | 暦を開いたとき、閉じたとき |
| `ItemView::is_editing()` | `editing` | 編集を始めたとき、終えたとき(モデルへ書き込む前) |
| `Menu::is_open()` | `opened` | 開いたとき(popup・メニューバー・親のメニュー)、閉じたとき |

引数を取るもの(`ItemView::is_selected(index)`・`TreeView::is_expanded(index)`)は関数のままにする。

### 名前を変えないもの

- `ItemView::drag_enabled`・`drop_enabled`: `Widget::accepts_drops`(部品がドロップを受け取るか)と意味が違う(項目の間に落として
  並べ替え・移す)。名前は変えず、違いをリファレンスに書いた。
- `Canvas::scale()`・`scale_by()`: 変換の `translate`・`rotate` と揃えて `scale` を変換にする案もあったが、`Bitmap::scale()`・
  `TextLayout::scale()` が倍率を返すのと揃わなくなるので、変えない。

### デザイナー

- カタログ・生成するコード・見本を新しい名前にした。
- 定義ファイルを読むときに、古い名前(Label の `color`、Timer の `on_timer`)を新しい名前に読み替える
  (`designer/packages/core/src/dsl/renamed.ts`)。古い名前で書いたファイルもそのまま開け、保存すると新しい名前で書き出す。形式の版
  (`format_version`)は変えない。

## 影響

- 利用者のコードは、改名したメンバ・関数を新しい名前に直す必要がある(1.0 の前の変更)。
- Canvas は純粋仮想関数だけの抽象クラスで、描く関数を足すたびに仮想関数の表が変わり ABI が変わる。公開の Canvas を仮想関数を持たない
  具体的なクラスにし、実際の描画は内部の描画先に任せる作り直しを、続けて行う(別の ADR)。
