# 部品の使い方 {#widgets_guide}

[TOC]

よく使う部品の使い方です。どの部品も親を渡して `new` で作り、状態はプロパティで読み書きし、変化はハンドラで受け取ります(@ref basics)。
画面の画像はクラスのページと @ref gallery にあります。

## どの部品にもあるもの

| プロパティ | 内容 |
|---|---|
| `visible`・`enabled` | 表示するか・操作できるか。無効の部品は薄く描かれ、子孫も無効になる |
| `tooltip` | マウスを止めたときに出す説明 |
| `focus_policy`・`focused` | フォーカスの得方(`TAB`・`CLICK`・`TAB_AND_CLICK`・`NONE`)と、今フォーカスがあるか |
| `minimum_size`・`maximum_size` | レイアウトが決める大きさの下限・上限(@ref layout_guide) |
| `cursor` | マウスのカーソルの形 |
| `theme_override` | その部品と子孫のテーマの一部の上書き(@ref basics) |

- **Tab の順**は作った順です。Tab・Shift+Tab でフォーカスのある部品を移します。
- **ニーモニック**: 文字に `&` を付けると、その次の文字を Alt と一緒に押して部品を操作できます(`"保存(&S)"`。`&&` は `&` そのもの)。
  ラベルのニーモニックは、タブの順で次の部品(または `buddy` に決めた部品)へフォーカスを移します。

## ボタン

```cpp
tokotachi::Button* button_save = new tokotachi::Button(window_main, "保存(&S)");
button_save->style = tokotachi::ButtonStyle::PRIMARY;       // PRIMARY(強調)・SECONDARY(枠だけ)
button_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
button_save->on_click = [=](tokotachi::Object*) { ... };

tokotachi::CheckBox* check_box_agree = new tokotachi::CheckBox(window_main, "規約に同意する(&A)");
check_box_agree->on_change = [=](tokotachi::Object*) { button_save->enabled = check_box_agree->checked; };
```

ラジオボタンは、各ボタンの `group_value` を同じ `IntVariable` に結ぶと 1 つのグループになり、選んだボタンの `value` が変数に入ります。

```cpp
tokotachi::IntVariable* size = new tokotachi::IntVariable(window_main, 2);
tokotachi::RadioButton* radio_button_small = new tokotachi::RadioButton(window_main, "小(&1)", 1);
tokotachi::RadioButton* radio_button_large = new tokotachi::RadioButton(window_main, "大(&2)", 2);
tokotachi::bind(radio_button_small->group_value, size->value);
tokotachi::bind(radio_button_large->group_value, size->value);   // size が 2 なので radio_button_large が選ばれる
```

## 文字の入力

`LineEdit`(1 行)と `TextEdit`(複数行)は、IME・双方向のテキスト・切り取り・コピー・貼り付けに対応します。

```cpp
tokotachi::LineEdit* line_edit_name = new tokotachi::LineEdit(window_main);
line_edit_name->placeholder_text = "名前";
line_edit_name->on_change = [=](tokotachi::Object*) { ... };   // 入力のたびに
line_edit_name->on_submit = [=](tokotachi::Object*) { ... };   // Enter で確定した

tokotachi::TextEdit* text_edit_memo = new tokotachi::TextEdit(window_main);
text_edit_memo->word_wrap = true;
```

- 文字は `text`、選んでいる文字は `selected_text`、カーソルの位置は `cursor_position`(バイトの位置)で読みます。
- `select_all()`・`clear_selection()`・`cut()`・`copy()`・`paste()`・`insert(文字)` で操作します。
- 値を複数の部品で共有するときは、`text` を `StringVariable` に結びます。

## 選ぶ: ComboBox

```cpp
tokotachi::ComboBox* combo_box_size = new tokotachi::ComboBox(window_main);
combo_box_size->items = std::vector<std::string>{"小", "中", "大"};
combo_box_size->current_index = 1;                               // -1 なら選んでいない(placeholder_text を出す)
combo_box_size->on_change = [=](tokotachi::Object*) { std::string s = combo_box_size->current_text; };
```

## 数の値: SpinBox・Slider・ProgressBar

どれも `minimum`・`maximum`・`value`(`double`)を持ちます。同じ変数に結べば、値がそろいます。

```cpp
tokotachi::DoubleVariable* volume = new tokotachi::DoubleVariable(window_main, 30);
tokotachi::SpinBox* spin_box_volume = new tokotachi::SpinBox(window_main);
spin_box_volume->suffix = " %";                                 // 単位
tokotachi::Slider* slider_volume = new tokotachi::Slider(window_main);
slider_volume->tick_interval = 10;                             // 目盛り
tokotachi::bind(spin_box_volume->value, volume->value);
tokotachi::bind(slider_volume->value, volume->value);
```

- `SpinBox` は `decimals`(小数の桁)・`single_step`(矢印のキー・ボタンの刻み)・`prefix`・`suffix` を持ちます。
- `ProgressBar` は `indeterminate` で、進み具合の分からない処理を表します。

## 日付と色

```cpp
tokotachi::DateEdit* date_edit_due = new tokotachi::DateEdit(window_main);
date_edit_due->date = tokotachi::Date::today();
date_edit_due->format = "yyyy-MM-dd";                          // 既定は "yyyy/MM/dd"

tokotachi::ColorButton* color_button_line = new tokotachi::ColorButton(window_main);
color_button_line->color = tokotachi::Color::rgb(0x2563eb);    // 押すと ColorDialog を開いて選ばせる
color_button_line->alpha_enabled = true;                       // 不透明度も選ばせる
```

- `DateEdit` は押すと暦(`Calendar`)を開きます。`minimum`・`maximum` で選べる範囲を決めます。日付の無い状態は `Date()`(`is_null()`)。
- 暦を画面に置くときは `Calendar` を使います。

## 表示: Label・ImageBox・GroupBox

```cpp
tokotachi::Label* label_status = new tokotachi::Label(window_main, "準備完了");
label_status->text_color = tokotachi::ColorRole::TEXT_SECONDARY;
label_status->word_wrap = true;                                // 幅で折り返す
label_status->icon = tokotachi::Image::icon(tokotachi::Icon::INFO);
```

- 画像だけを出すときは `ImageBox`(@ref drawing_guide)。
- 見出しの付いた枠は `GroupBox`(@ref layout_guide)。

## 入れ物

タブ(`TabWidget`)・分割(`Splitter`)・スクロール(`ScrollArea`)は @ref layout_guide 、一覧・木・表は @ref views_guide 、
自由に描く部品(`PaintBox`)は @ref drawing_guide を参照してください。
