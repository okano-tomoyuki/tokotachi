# 0058. クラスの階層を整理し、利用者が派生してよいクラス(拡張点)と標準の見た目を描く関数を決める

- 状態: 承認
- 日付: 2026-10-12

## 背景

[ADR 0057](0057-compatibility-abi-and-c-layer.md) で、公開のクラスを pimpl にし、1.0 から互換を約束することにした。pimpl で非公開の
データは隠せるが、次の問題が残る。

- **どこまで派生してよいかが決まっていない**: すべての部品が派生でき、利用者が Button の `paint` を上書きすることも、内部の都合で
  組んだ中間の基底(ButtonBase・TextInputBase・ItemView など)から派生することもできる。派生してよいクラスの protected の API と
  仮想関数の表は互換の約束の対象になるので、意図しない派生を許すと、内部を変えられなくなる。
- **内部のための仮想関数が公開されている**: Widget の protected の仮想関数には、利用者が上書きする口(`paint`・`mouse_down` など)と、
  組み込みの部品どうしの都合のためのもの(`child_removed`・`action_added`・`mnemonic_key`・`handles_tab_key`・`input_method_rect` など)が
  混ざっている。後者は利用者が上書きすると壊れうるうえ、足すたびに ABI が変わる。
- **自作の部品の敷居**: 完成した部品を閉じると、「ボタンに印を足したい」だけでも、押したときの振る舞い・キーボード・見た目を
  Widget から書くことになる。
- **階層の食い違い**: ScrollBar だけが RangeBase ではなく Widget から派生し、値の型も `float`(RangeBase は `double`)で、最小値を持たない。

Qt と C++Builder(VCL)の階層と比べると、大筋は同じ形をしている。

| Tokotachi | Qt | VCL |
|---|---|---|
| Object | QObject | TComponent |
| Widget | QWidget | TControl・TWinControl |
| Window ← Widget、Dialog ← Window | QWidget(トップレベル)・QDialog | TForm |
| ButtonBase ← Button・CheckBox・RadioButton・ColorButton | QAbstractButton | TButtonControl ← TCustomButton ← TButton |
| RangeBase ← Slider・SpinBox | QAbstractSlider ← QSlider・QScrollBar | TTrackBar・TScrollBar・TSpinEdit は別々 |
| TextInputBase ← LineEdit・TextEdit | QLineEdit・QTextEdit は別々 | TCustomEdit ← TEdit・TMemo |
| ItemView ← ListView・TreeView・TableView | QAbstractItemView | 別々 |
| FileDialog ← Object、ColorDialog・MessageDialog ← Dialog | どれも QDialog | どれも TCommonDialog(TComponent) |

Qt・VCL は、どちらも**振る舞いだけを持つ基底を、派生してよい段にしている**(Qt の QAbstractButton は「派生するなら paintEvent だけ
書けばよい」とし、VCL の TCustomButton・TCustomControl も部品を作る人のための段)。また、**標準の見た目を描く部品を公開している**
(Qt の `QStyle::drawControl`、VCL の `StyleServices.DrawElement`)ので、自作の部品も「標準のボタンの面を描いてから、印を足す」と書ける。
内部の都合は、Qt は private のクラスに、VCL はメッセージに置いている。

## 検討した選択肢

- 選択肢A: 今のまま、すべてを派生できるようにし、内部のための仮想関数もリファレンスで `@internal` と書くだけにする。
  - 作業は少ないが、誤用と ABI の弱点が残る。
- 選択肢B: 完成した部品は `final` にし、振る舞いだけを持つ基底(ButtonBase・RangeBase)を拡張点として開く。
  - 誤用は防げるが、完成した部品にプロパティ・ハンドラを足す派生ができなくなる。
    デザイナーの自作部品([ADR 0044](0044-designer-custom-widgets.md))は、`class LevelBar : public tokotachi::ProgressBar` のように完成した部品から派生してプロパティを足し、キャンバスでは基底の部品として
    描く作りなので、これが壊れる。ギャラリーの表のサンプルも、ListModel から派生して `set_text` を上書きしている。
- 選択肢C: 内部のための仮想関数を公開の仮想関数の表から外し(非公開のデータは ADR 0057 の pimpl で隠れる)、利用者から見えるものを
  公開の API と利用者向けの口だけにする。そのうえで、完成した部品も派生してよいものとし、振る舞いだけを持つ基底を拡張点として開き、
  標準の見た目を描く関数を公開する。`final` は、ライブラリが作るクラスにだけ付ける。
  - 利用者が派生して触れられるのは、もともと互換を約束する範囲(公開の API と利用者向けの口)だけになり、誤用と ABI の弱点は
    `final` に頼らずに防げる。完成した部品からの派生も使えるので、敷居も低い(Button から派生し、`Button::paint` を呼んでから印を足す。
    Qt と同じ)。

## 決定

**選択肢C を採る。**

### 拡張点(利用者が派生して部品を作る土台)

完成した部品のほかに、次のクラスを、利用者が派生して自分の部品・モデルなどを作る土台とする。

| クラス | 使いみち | 上書きする口 |
|---|---|---|
| Object | プロパティを持つ自分のクラス | (`notify_changed` で変更を知らせる) |
| Widget | 何でも自分で描く部品・自分で子を並べる入れ物 | 下の「Widget の protected」 |
| ButtonBase | 押して使う部品(押す・ホバー・クリック・キーボード・ニーモニックの振る舞いを持つ。利用者は描くだけ) | `paint`・`click` |
| RangeBase | 範囲の中の値を変える部品(つまみなど。値・範囲・刻み・キーとホイールの操作を持つ) | `paint`・`range_changed` |
| PaintBox | 自由に描く部品(ハンドラでも、派生でも描ける) | `paint` |
| Window・Dialog | 画面(デザイナーが生成するクラスもこれから派生する) | 入力のイベントなど |
| Popup | 自分のポップアップ | `paint` |
| Layout | 自分の並べ方 | `compute_preferred_size`・`compute_minimum_size`・`arrange`・`widget_removed` |
| ItemModel | 自分のデータの一覧・木のモデル | 項目の数・値などの仮想関数 |

- 拡張点の protected の API と仮想関数は、1.0 から互換の約束(ADR 0057)の対象にする。リファレンスに、派生するときに上書きする口と
  使ってよい関数を書く。
- ButtonBase の protected の API を、利用者向けに整える(押されているか・ホバーしているかなどの状態をまとめて返す `state()`、
  文字のレイアウト、ニーモニックの下線の描画)。

### 完成した部品

- 完成した部品(Button・ProgressBar・ListView・ListModel・Window など)は、派生してよい。プロパティ・ハンドラを足す(デザイナーの
  自作部品)、利用者向けの口を上書きする(基底の関数を呼んでから足す)、という使い方を認める。
- 完成した部品が上書きする仮想関数(`Button::paint` など)は、派生したクラスから呼べるよう protected に残す。完成した部品の protected には、
  Widget などの利用者向けの口の上書きだけを置き、内部向けの補助の関数は置かない(実装のクラスに移す)。
- `final` は、ライブラリが作って返すクラス(Tab・Column・SplitterPane・TreeItem など。利用者が派生しても、ライブラリが作るのは基底の型
  なので意味がない)にだけ付ける。
- `final` は後から外しても互換を壊さないが、後から付けると壊す。迷うものは今回の方針(派生してよい)を基本とし、ライブラリが作るクラスを
  足したときに `final` を付け忘れないようにする。

### 拡張点でない中間の基底

- TextInputBase・ItemView・Item は、型としては公開する(`ItemView*` で一覧・木・表をまとめて扱う、など)が、直接の派生は内部だけにする
  (それを継いだ完成した部品 LineEdit・ListView などからは派生してよい)。
  コンストラクタに内部の印の型(公開のヘッダでは不完全型)を取らせ、利用者が派生できないようにする。
- 一覧・木・表の見た目は、派生ではなく、項目ごとに描くハンドラで変えられるようにする方向とする(1.0 の後。VCL の OwnerDraw・
  Qt の delegate に当たる)。

### Widget の protected

利用者向けの口だけを protected の仮想関数に残す。

- 描画と大きさ: `paint`・`resized`・`compute_preferred_size`・`compute_minimum_size`・`arrange_children`(レイアウトを使わずに子を並べる)
- 入力: `mouse_down`・`mouse_up`・`mouse_move`・`mouse_enter`・`mouse_leave`・`mouse_wheel`・`key_down`・`key_up`・`text_input`・
  `text_editing`・`focus_in`・`focus_out`
- ドラッグ&ドロップ: `drag_move`・`drag_leave`・`drop`・`drag_finished`
- 状態の変化: `scale_changed`・`theme_changed`

内部のためのもの(`child_removed`・`action_added`・`action_removed`・`action_changed`・`mnemonic_key`・`mnemonic_activated`・
`handles_tab_key`・`input_method_rect`・`update_input_method`・`mark_layout_dirty`・`mnemonic_cues` など)は、公開のヘッダから外し、
`Widget::Impl` が持つ内部のフックに移す(部品の実装のクラスが、内部のフックの仮想関数を実装する)。内部のフックはヘッダに出ないので、
後から足しても ABI は変わらない。Object の `change_owner` も内部に移す。

自作の文字の入力の部品(`input_method_rect` などを使うもの)は、1.0 では対象外とする(必要になったら口を開く)。

### 標準の見た目を描く関数

- 組み込みの部品の見た目を描く関数を公開する(名前は実装のときに決める。例: `tokotachi::draw::button(canvas, rect, state, style)`)。
  今のテーマの色・角の丸み・倍率で描く。
- 組み込みの部品も、この関数で自分を描く(見た目の定義を 1 か所にまとめ、自作の部品も同じ見た目になる)。
- 1.0 では、ボタンの面・チェックの印・ラジオの印・フォーカスの枠・入力欄の枠を用意する。関数を足すのは互換を壊さないので、
  ほかの部品の見た目(スライダーの溝とつまみなど)は 1.0 の後に足してよい。

### 階層の食い違い

- ScrollBar を RangeBase から派生させる(Qt と同じ)。値の型を `double` にし、最小値を持つ。ScrollArea・ItemView の中の使い方も合わせる。
- ダイアログは、OS のダイアログ(FileDialog)を Window にできないので、階層はそのままにし、開き方の API(`show` と `on_finish`)を揃える。

## 影響

- ADR 0057 の pimpl の作業(手順 3)で、クラスごとに pimpl・内部のフックへの移し替え・内部向けの補助の関数の実装のクラスへの移動・
  (ライブラリが作るクラスへの)`final` をまとめて行う。
- デザイナーのカタログ(パレットの部品の基底)、リファレンスの分類([ADR 0056](0056-reference-class-docs-and-widget-images.md))、
  手引きに「自作の部品の作り方」(Widget・ButtonBase・RangeBase・PaintBox のどれから作るか、標準の見た目を描く関数)を加える。
- 完成した部品から派生している利用者のコード(デザイナーの自作部品など)はそのまま使える。内部のための仮想関数(`child_removed` など)を
  上書きしているコードは直す必要がある(1.0 の前の変更)。

## 実装(2026-10-12)

- **内部のフック**: `detail::WidgetHooks`(`src/widgets/widget_access.hpp`)。部品の実装のクラスが継いで上書きし、部品のコンストラクタで
  `install` する。前に差し込まれていたもの(基底の部品の分)は `base_` で呼べ、実装のクラスが破棄されると自分を外す(コンストラクタ・
  デストラクタの中で基底の振る舞いになるのは、仮想関数と同じ)。`mark_layout_dirty`・`update_input_method`・`mnemonic_cues` は
  Widget の実装に移し、内部からは `WidgetAccess` を通して呼ぶ。
- **入力欄**: LineEdit・TextEdit が上書きしていた `text_area`・`wrap_width`・`submit`・`scroll_changed` は `detail::TextInputHooks`
  (`src/widgets/text_input_impl.hpp`)に、`paint_text` などの補助は TextInputBase の実装に移した。
- **一覧・木・表**: ItemView の内部の関数(`expand`・表の選択・`copy_cells`・`cells_changed` など)は実装に移し、TreeView・TableView の
  公開の関数は窓口を通して呼ぶ。ListModel・TreeModel が継いでいた `detail::ItemHost` は、実装のクラスが継ぐ(定義も内部へ)。
- **派生の制限**: ItemView・TextInputBase のコンストラクタは `const detail::InternalTag&` を取る(公開のヘッダでは不完全型。
  ライブラリの中では `detail::internal_tag()` を渡す)。
- **標準の見た目**: `<tokotachi/widgets/draw.hpp>` の `tokotachi::draw::button_face`・`button_text_color`・`check_mark`・`radio_mark`・
  `focus_ring`・`field_frame` と、状態をまとめた `ControlState`(`<tokotachi/widgets/control_state.hpp>`)。`ButtonBase::state()` を
  足した。組み込みの部品も同じ関数で描き、リファレンスの部品の画像が前と画素まで同じことを確かめた。
- **手引き**: 「自作の部品」(`docs/guide/custom_widgets.md`)を加えた。中のコード例は、シンボルを隠した共有ライブラリに対して
  コンパイル・リンクできることを確かめた。
