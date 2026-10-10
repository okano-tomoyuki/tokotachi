# レイアウト {#layout_guide}

[TOC]

部品の位置と大きさは、親に付けた**レイアウト**が決めます。レイアウトは各部品の望む大きさ(`preferred_size`)と最小の大きさを見て並べ、
ウィンドウの大きさが変わると並べ直します。

## BoxLayout

縦または横に一列に並べます。

```cpp
tokotachi::BoxLayout* column = new tokotachi::BoxLayout(&window);                       // 縦
column->margins = tokotachi::Margins(16);
column->spacing = 8;
column->add(title);                                   // 望む大きさで
column->add(memo, 1);                                 // stretch 1: 余った高さを受け持つ
column->add(ok, 0, tokotachi::Align::END);            // 横は右(終わり)に寄せる
column->add_stretch();                                // 余白
```

横に並べる行は、入れ物の部品(`Widget`)に横の BoxLayout を付け、その部品を縦の BoxLayout に入れます(入れ子)。

## GridLayout

表のように行と列に並べます。項目名と入力欄のフォームに向いています。

```cpp
tokotachi::GridLayout* grid = new tokotachi::GridLayout(&window);
grid->add(new tokotachi::Label(&window, "名前"), 0, 0);
grid->add(name, 0, 1);
grid->add(note, 1, 0, 1, 2);          // 1 行目から 1 行・2 列にまたがる
grid->set_column_stretch(1, 1);       // 2 列目が幅を受け持つ
```

## PlaceLayout

親の大きさに対する割合や、端からの距離で置きます(重ねる・隅に置く)。

## スクロールと分割

- `ScrollArea`: 中身が見える範囲より大きいとき、スクロールして見せます(`content` に中身を代入)。
- `Splitter`: 区画を並べ、境目をドラッグして大きさを変えます。`add_widget(部品, stretch)` の stretch 0 の区画は、ウィンドウの大きさが
  変わっても大きさを保ちます。区画は端まで寄せるとたためます。`sizes` で大きさを保存・復元できます。

```cpp
tokotachi::Splitter* split = new tokotachi::Splitter(&window);
split->add_widget(tree, 0);
split->add_widget(list, 1);
split->sizes = std::vector<float>{200, 400};
```

## 大きさの指定

- `minimum_size`・`maximum_size` で、レイアウトに縮めて・広げてよい範囲を決められます。
- 部品が自分で決める最小の大きさ(文字が切れない大きさなど)との大きい方が使われます(`minimum_hint`)。
- 文字が変わるなどで望む大きさが変わると、自動で並べ直します。
