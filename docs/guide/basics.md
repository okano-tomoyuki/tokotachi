# 基本の考え方 {#basics}

[TOC]

## 親子と所有

部品(Widget)は、親を渡して `new` で作ります。**親が子を所有し**、親が破棄されると子もすべて破棄されます。

```cpp
tokotachi::Widget* panel = new tokotachi::Widget(&window);
tokotachi::Button* ok = new tokotachi::Button(panel, "OK");   // panel が所有する
delete panel;                                                // ok も一緒に破棄される
```

- `parent` プロパティに代入すると、親(と所有者)を付け替えられます。
- 親を `nullptr` で作った部品は、レイアウトに `add` する・タブに加えるなどで、そこの子になります。
- タイマー・変数・モデル・Action などの部品でないもの(`tokotachi::Object`)も、`owner` を渡すと所有されます。
- 自分のハンドラの中で自分を破棄したいときは `delete_later()` を使います(イベントの処理が終わってから破棄します)。

## プロパティ

状態は getter・setter の組ではなく、**プロパティ**で読み書きします。

```cpp
label->text = "こんにちは";               // 書き込み
std::string s = label->text;             // 読み取り(暗黙の変換)
label->text.get().size();                // 値のメソッドを呼ぶときは get()
button->x += 10;                         // 複合代入
```

読み取り専用のプロパティ(`ReadOnlyProperty`)は代入できません(例: `Widget::children`・`ListView::selection`)。

## ハンドラ

イベントは、部品ごとに 1 つのハンドラ(`on_xxx` プロパティ)で受け取ります。**最初の引数はいつも発生元**です。

```cpp
button->on_click = [](tokotachi::Object* sender) { ... };
edit->on_key_down = [](tokotachi::Object* sender, tokotachi::KeyEvent& e) { ... };
```

1 つのハンドラを複数の部品で共有し、`sender` で区別できます。ハンドラを外すときは空の関数を代入します。

自作の部品(Widget の派生クラス)は、`paint`・`mouse_down`・`key_down` などの protected の仮想関数を上書きします。

## 変数と結びつけ

画面の状態は**変数**(`StringVariable`・`IntVariable`・`DoubleVariable`・`BoolVariable` など)に持ち、部品のプロパティと**結びつけ**ます。
どちらを変えても、もう片方に伝わります。

```cpp
tokotachi::StringVariable* name = new tokotachi::StringVariable(&window);
tokotachi::bind(edit->text, name->value);          // 入力欄と変数を結ぶ
name->on_change = [=](tokotachi::Object*) { ... }; // 変数が変わった(入力でも代入でも)
name->value = "初期値";                             // 入力欄にも出る
```

- 型が同じプロパティどうしを結べます(部品どうしでも)。
- ラジオボタンは、各ボタンの `group_value` を同じ `IntVariable` に結ぶと 1 つのグループになります。
- どちらかが破棄されると、結びつきは自動で外れます。

## テーマ

色は Application のテーマで決まり、既定では OS のライト・ダークの設定に従います。

```cpp
app.color_scheme = tokotachi::ColorScheme::DARK;       // SYSTEM・LIGHT・DARK
label->color = tokotachi::ColorRole::TEXT_SECONDARY;   // 役割で指定すると、テーマを替えたときに追従する
label->color = tokotachi::Color::rgb(0xdc2626);        // 具体的な色も指定できる
```

## 座標

- 単位は dp。位置は親の左上からの相対です。
- `bounds`・`x`・`y`・`width`・`height` で決められますが、ふつうはレイアウト(@ref layout_guide)に任せます。
