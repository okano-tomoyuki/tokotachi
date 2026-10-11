# カタログ

デザイナーが扱う部品・レイアウト・オブジェクトの一覧と、それぞれのプロパティ・ハンドラの情報。
定義ファイルの検証([dsl-spec.md](dsl-spec.md) §7)、デザイナーのパレットとインスペクタ、コード生成(プロパティの型・ハンドラの引数)、
キャンバスの描画の部分(名前から部品を作り、プロパティを設定する対応表)が使う。

**公開のヘッダから Doxygen の XML 出力で取り出した情報(型)と、実物での測定(既定値)と、手書きの補足(デザイン時の扱い)の 3 つを重ねて作る**
(Bethany と同じ考え方。Bethany はヘッダを自前で解析し、既定値を Python のバインディングで測った)。

## 1. 情報の出どころ

| 情報 | 出どころ |
|---|---|
| クラスの一覧と継承の関係、コンストラクタの引数 | Doxygen の XML(`compounddef`・`basecompoundref`・`memberdef kind="function"`) |
| プロパティの名前・型・読み取り専用か・宣言したクラス・説明 | 同上(`Property<T>`・`ReadOnlyProperty<T>` の public のメンバ。`///` の説明は `briefdescription`) |
| ハンドラの名前と型、その型の引数 | 同上(`Property<NotifyHandler>` 等のメンバと、`typedef std::function<void(Object* sender, ...)> XxxHandler`) |
| 列挙型の値とその説明 | 同上(`memberdef kind="enum"`) |
| 値の型(`Margins`・`Size`・`Font`・`Color`・`Placement` 等)のメンバ | 同上(struct の public のメンバ) |
| 各プロパティの既定値、部品の望む大きさ | 実物で測る(§2 の 2) |
| デザイン時に設定できるか、パレットの分類と順、子を持てるか、専用の子のキー、インスペクタの編集の種類 | 手書きの補足(§3) |

Doxygen の XML には、宣言の順(カタログの順になる)・`std::vector<std::string>` のような入れ子の型・ハンドラの typedef の中身が
そのまま入ることを確かめた(2026-10-10、Doxygen 1.18.0)。`tokotachi::detail` は含めない(リファレンスと同じ)。

## 2. 取り出しの手順

`designer/tools/catalog/`(TypeScript。Node.js で動かす。デザイナーの開発にはもともと Node.js が要るので、Python は要らない)。
作ったカタログ(`designer/packages/core/src/catalog/catalog.json`)はコミットする。

1. **型を取り出す**: 公開のヘッダから Doxygen の XML を作り(取り出しの道具が XML だけを作る設定で Doxygen を動かす。CMake の設定は要らない)、
   クラス・プロパティ・ハンドラ・列挙型・値の型を取り出す。
2. **既定値を測る**: 取り出した型から、キャンバスの描画の部分が使う「名前から部品を作る・プロパティを読み書きする」C++ の対応表を生成し
   ([ADR 0039](../adr/0039-designer.md))、それを使う測定用のプログラムで、各クラスを作って各プロパティの値と望む大きさを読む。
   キャンバスと同じ WebAssembly(Node.js)で動かせるので、OS に依らない値が取れる(フォントで変わる望む大きさは、Windows の値を基準にする)。
3. **補足を重ねる**: 手書きの補足(`designer/tools/catalog/overlay.json`)を重ねて、最終的なカタログにする。

1 と 3 は [designer/tools/catalog/extract.mts](../../designer/tools/catalog/extract.mts)、2 は [measure.mts](../../designer/tools/catalog/measure.mts)
(`pnpm catalog:measure`。キャンバスのモジュールが要る。[ADR 0040](../adr/0040-designer-canvas-ops.md))。
測った値はプロパティの `default` と、部品の `preferred_size` に書き、取り出し直しても引き継ぐ。

```sh
pnpm catalog:extract   # 1 と 3(Doxygen が要る。測った既定値は前のカタログから引き継ぐ)
pnpm catalog:measure   # 2(先に pnpm canvas:build)
pnpm catalog:check     # カタログがヘッダ・補足と食い違っていればエラー
```

## 3. 手書きの補足

| 項目 | 内容 | 例 |
|---|---|---|
| `classes` | パレットに出す、利用者が置けるクラスの一覧(抽象的なクラス・内部で使うクラスは書かない)。無いクラス・余分なクラスがあれば取り出しの道具がエラーにする | `Button`・`LineEdit`・`TabWidget`。`ButtonBase`・`RangeBase`・`ItemView`・`Popup` は書かない |
| `classes` のテンプレート | `Variable<T>` は `part` として書き、ハンドラ(`on_set`・`on_change`)だけをカタログに載せる。テンプレートであることは Doxygen の XML から取り出し(`template: true`)、キャンバスの対応表には載せない(型の引数は定義ファイルの `type`。dsl-spec.md §4.8) | `Variable` |
| `classes` の `kind` / `category` | 種類(window・widget・layout・part・object)とパレットの分類。`classes` の順がパレットの順 | 入力(`Button`・`CheckBox`…)・表示(`Label`・`ProgressBar`・`ImageBox`)・一覧(`ListView`・`TreeView`)・入れ物(`Widget`・`TabWidget`・`Splitter`・`ScrollArea`)・描画(`PaintBox`)・画面に出ないもの(`Timer`) |
| `designable` / `not_designable` | デザイン時に設定できるか。まず型で決め(読み書きできて、値の型・列挙型・文字列・配列・参照のいずれか)、そのうえでクラスごとに除く・戻す | `parent`・`bounds`・`focused` は除く(位置と大きさは `geometry` の印で表す) |
| `accepts_children` | `layout`・`children` を書けるか | `Widget`・`Window`・`Dialog` は可、`Button` は不可 |
| `child_keys` | 専用の子のキー | `TabWidget` は `tabs`、`Splitter` は `panes`、`ScrollArea` は `content`、`ListView`・`TreeView` は `columns` |
| `editor` | インスペクタの編集の種類(型から決まらないもの) | `TextEdit` の `text` は複数行、`Action` の `text` は `&` を含む文字列、`Label` の `color` は色 |
| `excluded` | Widget の派生で親を渡して作れるが、パレットに出さないクラス(理由を書く)。classes にも excluded にも無い部品があれば、取り出しの道具がエラーにする(新しい部品の書き忘れを防ぐ) | `ButtonBase`・`ScrollBar` |
| `geometry` | 位置と大きさのプロパティ(ウィンドウと自由な配置の子だけに書ける) | `Widget` の `x`・`y`・`width`・`height` |
| `after_children` | 子(タブ・区画)を入れた後でないと効かないプロパティ(生成するコードでは子の後に設定する。codegen-design.md §4) | `TabWidget::current_index`・`Splitter::sizes` |
| `range` | 数値のプロパティの範囲(ヘッダの説明に書いたもの)。端は数値、同じ部品のプロパティ(`{ "property": "maximum" }`)、配列のプロパティ・子の数に offset を足したもの(`{ "count": "items", "offset": -1 }`)。インスペクタが確定した値を収め、検証が範囲の外を警告する | `RangeBase::value` は minimum〜maximum、`SpinBox::decimals` は 0〜10、`Column::width` は 24 以上 |
| `extra_enums`・`extra_values` | プロパティの型には出てこないが、定義ファイルで使う列挙型・値の型 | `Align`・`Anchor`・`DialogButton`・`ColorRole`・`Placement`・`Color` |
| `special_values` | メンバではなく専用の書き方で表す値の型 | `ThemeColor`・`Shortcut`・`Image`(パスで書く。dsl-spec.md §5.1) |

インスペクタの並びは宣言の順(カタログの順)のままにする。宣言の順で設定すれば正しく働くことを確かめたので(範囲の `minimum`・`maximum` が `value` の前、等)、並びを変える補足は今は持たない。

## 4. カタログの形(案)

```json
{
  "classes": {
    "Button": {
      "bases": ["ButtonBase", "Widget", "Object"],
      "kind": "widget",
      "category": "input",
      "preferred_size": { "width": 60, "height": 36 },
      "doc": "押すと on_click を呼ぶボタン。",
      "properties": {
        "text": { "type": "string", "declared_in": "ButtonBase", "default": "" },
        "style": { "type": { "enum": "ButtonStyle" }, "declared_in": "Button", "default": "PRIMARY" },
        "font": { "type": { "value": "Font" }, "declared_in": "ButtonBase", "default": { "size": 14 } },
        "enabled": { "type": "bool", "declared_in": "Widget", "default": true }
      },
      "handlers": {
        "on_click": { "type": "NotifyHandler", "declared_in": "ButtonBase" }
      }
    }
  },
  "values": {
    "Margins": { "members": { "left": "float", "top": "float", "right": "float", "bottom": "float" } },
    "Font": { "members": { "size": "float" } }
  },
  "enums": {
    "Orientation": { "values": ["HORIZONTAL", "VERTICAL"] },
    "ButtonStyle": { "values": ["PRIMARY", "SECONDARY"], "docs": { "PRIMARY": "…", "SECONDARY": "…" } }
  },
  "handler_types": {
    "NotifyHandler": [{ "name": "sender", "type": "Object*" }],
    "CloseHandler": [{ "name": "sender", "type": "Object*" }, { "name": "can_close", "type": "bool&" }]
  }
}
```

値(既定値・大きさ)は形を示すための例で、実際の値は測定で決まる。
継承したプロパティ・ハンドラは、各クラスに展開して持つ(検証・インスペクタで継承をたどらずに済むように)。
デザイン時に設定できないプロパティは書き出さない。`doc` はヘッダの `///` の説明(日本語)。

## 4.1 自作部品([ADR 0044](../adr/0044-designer-custom-widgets.md))

利用者が C++ で書いた部品は、プロジェクトファイルの `custom_widgets` に登録したヘッダを拡張が読み(core の
[customHeader.ts](../../designer/packages/core/src/catalog/customHeader.ts)。Doxygen は使わない)、カタログに足す(`findClass` が引ける)。

- 読むのは `class X : public Base` の形の、ライブラリの部品(か、登録した自作部品)の派生クラスと、その public の `Property<T> name;`。
  T がカタログの型ならプロパティ、ハンドラの型ならハンドラにする。読めない型は飛ばし、プロジェクトファイルの問題(警告)として出す。
  直前の `///`(行の後ろの `///<`)を説明に、`{3}`・`= 3` の文字の既定値を既定値にする。
- クラスの情報は `kind: "widget"`・`category: "custom"` で、基底のプロパティ・ハンドラを受け継ぐ。`custom` に、ヘッダ・名前空間を付けた名前・
  キャンバスで代わりに作るカタログの部品(`base`)を持つ。子は持てない。
- 拡張はいくつもの定義ファイルを扱うので、検証・書き出し・コード生成・描画の間だけ `withCustomClasses` で足す。Webview は `customClasses` の
  メッセージで受け取って `setCustomClasses` で足し、検証し直す。

## 5. ヘッダとの整合

公開のヘッダを変えたら、カタログを取り出し直す。
型と補足の部分は Doxygen だけで取り出せるので、`catalog:check` で取り出し直した結果とコミットされたカタログを比べ、
食い違っていればエラーにする(デザイナーの CI と、ヘッダを変える開発者の確認で使う)。既定値の測り直しは手で行う。

## 6. 未決の論点

| # | 論点 | 案 |
|---|---|---|
| C1 | プロパティの範囲(`Slider` の `value` は `minimum`〜`maximum`)の検証 | 当面は検証しない(実行時に Tokotachi が丸める)。インスペクタでは、同じ部品の範囲を見て入力を制限する |
| C2 | 型からは分からない制約(`current_index` は `items` の数より小さい等) | 補足に書かず、実行時の動き(キャンバスで実際に組んだ結果)に任せる |
| C3 | 利用者が作った部品(Widget の派生) | MVP の後。利用者のヘッダから同じ手順でカタログを足せるようにする案。キャンバスには仮の枠で表示する |
