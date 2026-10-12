# レイアウト {#layout_guide}

[TOC]

部品の位置と大きさは、親に付けた**レイアウト**が決めます。レイアウトは各部品の望む大きさ(`preferred_size`)と最小の大きさを見て並べ、
ウィンドウの大きさが変わると並べ直します。

## BoxLayout

縦または横に一列に並べます。

```cpp
tokotachi::BoxLayout* column = new tokotachi::BoxLayout(window_main);                       // 縦
column->margins = tokotachi::Margins(16);
column->spacing = 8;
column->add(label_title);                                   // 望む大きさで
column->add(text_edit_memo, 1);                                 // stretch 1: 余った高さを受け持つ
column->add(button_ok, 0, tokotachi::Align::END);            // 横は右(終わり)に寄せる
column->add_stretch();                                // 余白
```

横に並べる行は、入れ物の部品(`Widget`)に横の BoxLayout を付け、その部品を縦の BoxLayout に入れます(入れ子)。

## GridLayout

表のように行と列に並べます。項目名と入力欄のフォームに向いています。

```cpp
tokotachi::GridLayout* grid = new tokotachi::GridLayout(window_main);
grid->add(new tokotachi::Label(window_main, "名前"), 0, 0);
grid->add(line_edit_name, 0, 1);
grid->add(text_edit_note, 1, 0, 1, 2);          // 1 行目から 1 行・2 列にまたがる
grid->set_column_stretch(1, 1);       // 2 列目が幅を受け持つ
```

## PlaceLayout

親の大きさに対する割合や、端からの距離で置きます(重ねる・隅に置く)。

## 見出しの付いた枠

`GroupBox` は、関係する部品を見出しの付いた枠でまとめます。付けたレイアウトは、枠と見出しの内側(`content_rect`)に並べます。

```cpp
tokotachi::GroupBox* group_box_output = new tokotachi::GroupBox(window_main, "出力(&O)");
tokotachi::BoxLayout* box_output = new tokotachi::BoxLayout(group_box_output);
box_output->add(new tokotachi::CheckBox(group_box_output, "上書きする"));
```

## スクロールと分割

- `ScrollArea`: 中身が見える範囲より大きいとき、スクロールして見せます(`content` に中身を代入)。
- `Splitter`: 区画を並べ、境目をドラッグして大きさを変えます。`add_widget(部品, stretch)` の stretch 0 の区画は、ウィンドウの大きさが
  変わっても大きさを保ちます。区画は端まで寄せるとたためます。`sizes` で大きさを保存・復元できます。
  境目をダブルクリックすると、隣の区画をたたむ・戻します(`SplitterPane::collapsed` でも)。境目を押した後は、矢印キーで動かせます。

```cpp
tokotachi::Splitter* splitter_main = new tokotachi::Splitter(window_main);
splitter_main->add_widget(tree_view_folders, 0);
splitter_main->add_widget(list_view_files, 1);
splitter_main->sizes = std::vector<float>{200, 400};
```

## 大きさの指定

- `minimum_size`・`maximum_size` で、レイアウトに縮めて・広げてよい範囲を決められます。
- 部品が自分で決める最小の大きさ(文字が切れない大きさなど)との大きい方が使われます(`minimum_hint`)。
- 文字が変わるなどで望む大きさが変わると、自動で並べ直します。
