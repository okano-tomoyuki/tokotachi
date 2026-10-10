# カタログ(草案)

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

1. **型を取り出す**: 公開のヘッダから Doxygen の XML を作り(リファレンスと同じ Doxyfile に `GENERATE_XML` を加えた設定)、
   クラス・プロパティ・ハンドラ・列挙型・値の型を取り出す。
2. **既定値を測る**: 取り出した型から、キャンバスの描画の部分が使う「名前から部品を作る・プロパティを読み書きする」C++ の対応表を生成し
   ([ADR 0039](../adr/0039-designer.md))、それを使う測定用のプログラムで、各クラスを作って各プロパティの値と望む大きさを読む。
   キャンバスと同じ WebAssembly(Node.js)で動かせるので、OS に依らない値が取れる(フォントで変わる望む大きさは、Windows の値を基準にする)。
3. **補足を重ねる**: 手書きの補足(`designer/tools/catalog/overlay.json`)を重ねて、最終的なカタログにする。

```sh
pnpm catalog:extract   # 1〜3(Doxygen と、WebAssembly の Tokotachi のビルドが要る)
pnpm catalog:check     # カタログがヘッダ・補足と食い違っていればエラー(1 と 3 だけ。既定値は測り直さない)
```

## 3. 手書きの補足

| 項目 | 内容 | 例 |
|---|---|---|
| `classes` | パレットに出す、利用者が置けるクラスの一覧(抽象的なクラス・内部で使うクラスは書かない)。無いクラス・余分なクラスがあれば取り出しの道具がエラーにする | `Button`・`LineEdit`・`TabWidget`。`ButtonBase`・`RangeBase`・`ItemView`・`Popup` は書かない |
| `category` / `order` | パレットの分類と順 | 入力(`Button`・`CheckBox`…)・表示(`Label`・`ProgressBar`)・一覧(`ListView`・`TreeView`)・入れ物(`Widget`・`TabWidget`・`Splitter`・`ScrollArea`)・描画(`PaintBox`)・画面に出ないもの(`Timer`) |
| `designable` / `not_designable` | デザイン時に設定できるか。まず型で決め(読み書きできて、値の型・列挙型・文字列・配列・参照のいずれか)、そのうえでクラスごとに除く・戻す | `parent`・`bounds`・`focused` は除く。`x`・`y`・`width`・`height` は除き、自由な配置の子とウィンドウだけで戻す |
| `accepts_children` | `layout`・`children` を書けるか | `Widget`・`Window`・`Dialog` は可、`Button` は不可 |
| `child_keys` | 専用の子のキー | `TabWidget` は `tabs`、`Splitter` は `panes`、`ScrollArea` は `content` |
| `editor` | インスペクタの編集の種類(型から決まらないもの) | `TextEdit` の `text` は複数行、`Action` の `text` は `&` を含む文字列、`Label` の `color` は色 |
| `display_order` | インスペクタの並びと、生成するコードで設定する順(宣言の順と変えたいものだけ) | `Slider`・`SpinBox` の `value` を `minimum`・`maximum` の後に |

## 4. カタログの形(案)

```json
{
  "classes": {
    "Button": {
      "bases": ["ButtonBase", "Widget", "Object"],
      "kind": "widget",
      "category": "input",
      "accepts_children": false,
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
